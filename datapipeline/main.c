#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "veri.h"
#include "kurallar.h"
#include "sifreleme.h"
#include "buffer.h"
#include "yonlendirme.h"
#include "sinyal.h"
#include "hata.h"
#include "logrotasyon.h"

#define LOG_MAX_BOYUT  (10 * 1024 * 1024)   // 10 MB
#define MAKS_DENEME    3
#define BEKLEME_MS     100

// Global temizlik için (sinyal handler'dan erişilir)
static Hedef *g_kabulHedefi = NULL;
static Hedef *g_redHedefi = NULL;
static Kural *g_kurallar = NULL;
static int g_kuralSayisi = 0;

// Sinyal geldiğinde çağrılan temizlik (ana döngüde)
static void temizlik(void) {
    fprintf(stderr, "\n[BILGI] Sinyal alindi, kapatiliyor...\n");

    if (g_kabulHedefi) { hedefKapat(g_kabulHedefi); g_kabulHedefi = NULL; }
    if (g_redHedefi)   { hedefKapat(g_redHedefi);   g_redHedefi = NULL; }
    if (g_kurallar)    { kurallariTemizle(g_kurallar, g_kuralSayisi); g_kurallar = NULL; }

    fprintf(stderr, "[BILGI] Temizlik tamamlandi\n");
}

// Dinamik satır okuyucu
char *satirOku(FILE *fp) {
    Buffer *b = bufferOlustur(128);
    if (!b) return NULL;

    int c;
    int veriVar = 0;

    while ((c = fgetc(fp)) != EOF) {
        veriVar = 1;
        if (c == '\n') break;
        if (!bufferEkleChar(b, (char)c)) {
            bufferSil(b);
            return NULL;
        }
    }

    if (!veriVar) { bufferSil(b); return NULL; }

    char *sonuc = malloc(b->uzunluk + 1);
    if (!sonuc) { bufferSil(b); return NULL; }

    memcpy(sonuc, b->veri, b->uzunluk);
    sonuc[b->uzunluk] = '\0';

    bufferSil(b);
    return sonuc;
}

int main(void) {
    int satirNo = 0;
    int kabulSayisi = 0;
    int redSayisi = 0;

    // Sinyal yakalama — temizlik fonksiyonu ile
    if (!sinyalBaslat(temizlik)) {
        fprintf(stderr, "[HATA] Sinyal yakalama baslatılamadi\n");
        return 1;
    }

    // Log rotasyonu — önce kontrol et
    logRotasyon("red.log", LOG_MAX_BOYUT);
    logRotasyon("kabul.txt", LOG_MAX_BOYUT);

    // Kurallar
    Kural kurallar[MAX_KURAL] = {0};
    int kuralSayisi = 0;
    kurallariBaslat(kurallar, &kuralSayisi);

    g_kurallar = kurallar;
    g_kuralSayisi = kuralSayisi;

    // Hedefler
    Hedef *kabulHedefi = hedefAc(HEDEF_DOSYA, "kabul.txt");
    Hedef *redHedefi   = hedefAc(HEDEF_RED_LOG, "red.log");

    g_kabulHedefi = kabulHedefi;
    g_redHedefi = redHedefi;

    if (!kabulHedefi || !redHedefi) {
        fprintf(stderr, "[HATA] Hedefler acilamadi\n");
        temizlik();
        return 1;
    }

    fprintf(stderr, "[BILGI] %d kural yuklendi\n", kuralSayisi);
    fprintf(stderr, "[BILGI] Kabul -> kabul.txt, Red -> red.log\n");
    fprintf(stderr, "[BILGI] Log max boyut: %d MB\n\n", LOG_MAX_BOYUT / (1024 * 1024));

    const char *anahtar = "gizliAnahtar123";
    size_t anahtarUzunluk = strlen(anahtar);

    fprintf(stdout, "Veri gir (cikis: CTRL + D):\n");

    char *hamSatir;
    while ((hamSatir = satirOku(stdin)) != NULL) {
        // Sinyal kontrolü — her satırda
        if (sinyalGeldiMi()) {
            free(hamSatir);
            break;
        }

        satirNo++;

        Veri *v = veriAl(hamSatir, satirNo);
        free(hamSatir);

        if (v == NULL) {
            hataLogla(HATA_BELLEK, "main.c", __LINE__, NULL);
            continue;
        }

        trim(v->icerik);
        turkceAscii(v->icerik);
        kucukHarf(v->icerik);

        KontrolSonucu ks = veriKontrolEt(v->icerik, kurallar, kuralSayisi);
        if (ks.reddedildi) {
            redSayisi++;

            hedefeYaz(redHedefi,
                "[RED] Satir %d | Kural: %s | Konum: %d | Uzunluk: %d\n"
                "[RED]   Desen: %s\n"
                "[RED]   Eslesen: '%.*s'\n"
                "[RED]   Ham: [%s]\n\n",
                v->satirNo, ks.kuralAdi, ks.konum, ks.uzunluk,
                ks.desen, ks.uzunluk, v->icerik + ks.konum, v->icerik);

            fprintf(stdout, "[%d] REDDEDILDI\n", v->satirNo);
            veriSil(v);
            continue;
        }

        ozelKarakterTemizle(v->icerik);
        cokluBoslukSadelestir(v->icerik);
        trim(v->icerik);

        KelimeListesi *kl = kelimeListesiOlustur();
        if (!kl) { veriSil(v); continue; }

        char *kopya = strdup(v->icerik);
        char *token = strtok(kopya, " ");
        while (token) {
            kelimeEkle(kl, token);
            token = strtok(NULL, " ");
        }

        int toplam = sayilariTopla(v->icerik);

        char *acikKopya = strdup(v->icerik);
        if (!acikKopya) {
            free(kopya);
            kelimeListesiSil(kl);
            veriSil(v);
            continue;
        }

        size_t orijinalUzunluk = strlen(v->icerik);
        xorSifrele(v->icerik, orijinalUzunluk, anahtar, anahtarUzunluk);

        char *hex = hexKodla((unsigned char *)v->icerik, orijinalUzunluk);
        if (!hex) {
            hataLogla(HATA_BELLEK, "main.c", __LINE__, "hex kodlama");
            free(acikKopya);
            free(kopya);
            kelimeListesiSil(kl);
            veriSil(v);
            continue;
        }

        kabulSayisi++;

        hedefeYaz(kabulHedefi,
            "[%d] Sifreli: %s\n"
            "    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
            v->satirNo, hex, orijinalUzunluk, kl->sayi, toplam);

        printf("[%d] Sifreli: %s\n", v->satirNo, hex);
        printf("    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
               orijinalUzunluk, kl->sayi, toplam);

        fprintf(stderr, "[SIFRE] Satir %d | Acik: [%s]\n", v->satirNo, acikKopya);

        free(hex);
        free(acikKopya);
        free(kopya);
        kelimeListesiSil(kl);
        veriSil(v);

        // Her 1000 satırda log rotasyon kontrolü
        if (satirNo % 1000 == 0) {
            logRotasyon("red.log", LOG_MAX_BOYUT);
            logRotasyon("kabul.txt", LOG_MAX_BOYUT);
        }
    }

    // Özet
    fprintf(stdout, "\n=== OZET ===\n");
    fprintf(stdout, "Toplam: %d | Kabul: %d | Red: %d\n",
            satirNo, kabulSayisi, redSayisi);

    fprintf(stderr, "[BILGI] Toplam %d satir, %d kabul, %d red\n",
            satirNo, kabulSayisi, redSayisi);

    temizlik();
    return 0;
}