#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "veri.h"
#include "kurallar.h"
#include "sifreleme.h"
#include "buffer.h"
#include "yonlendirme.h"      

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

    Kural kurallar[MAX_KURAL] = {0};
    int kuralSayisi = 0;
    kurallariBaslat(kurallar, &kuralSayisi);

    Hedef *kabulHedefi = hedefAc(HEDEF_DOSYA, "kabul.txt");
    Hedef *redHedefi   = hedefAc(HEDEF_RED_LOG, "red.log");

    if (!kabulHedefi || !redHedefi) {
        fprintf(stderr, "[HATA] Hedefler acilamadi\n");
        if (kabulHedefi) hedefKapat(kabulHedefi);
        if (redHedefi) hedefKapat(redHedefi);
        kurallariTemizle(kurallar, kuralSayisi);
        return 1;
    }

    fprintf(stderr, "[BILGI] %d kural yuklendi\n", kuralSayisi);
    fprintf(stderr, "[BILGI] Kabul -> kabul.txt, Red -> red.log\n\n");

    const char *anahtar = "gizliAnahtar123";
    size_t anahtarUzunluk = strlen(anahtar);

    fprintf(stdout, "Veri gir (cikis: CTRL + D):\n");

    char *hamSatir;
    while ((hamSatir = satirOku(stdin)) != NULL) {
        satirNo++;

        Veri *v = veriAl(hamSatir, satirNo);
        free(hamSatir);

        if (v == NULL) {
            fprintf(stderr, "[HATA] Satir %d: bellek yetersiz\n", satirNo);
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

        // Dinamik açık kopya (MAX_SATIR yok)
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
            fprintf(stderr, "[HATA] Satir %d: sifreleme\n", v->satirNo);
            free(acikKopya);
            free(kopya);
            kelimeListesiSil(kl);
            veriSil(v);
            continue;
        }

        kabulSayisi++;

        // kabul.txt'ye yaz
        hedefeYaz(kabulHedefi,
            "[%d] Sifreli: %s\n"
            "    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
            v->satirNo, hex, orijinalUzunluk, kl->sayi, toplam);

        // Kullanıcıya da göster
        printf("[%d] Sifreli: %s\n", v->satirNo, hex);
        printf("    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
               orijinalUzunluk, kl->sayi, toplam);

        fprintf(stderr, "[SIFRE] Satir %d | Acik: [%s]\n", v->satirNo, acikKopya);
        fprintf(stderr, "[KABUL] Satir %d\n\n", v->satirNo);

        free(hex);
        free(acikKopya);
        free(kopya);
        kelimeListesiSil(kl);
        veriSil(v);
    }

    // Özet
    fprintf(stdout, "\n=== OZET ===\n");
    fprintf(stdout, "Toplam: %d | Kabul: %d | Red: %d\n",
            satirNo, kabulSayisi, redSayisi);

    fprintf(stderr, "[BILGI] Toplam %d satir, %d kabul, %d red\n",
            satirNo, kabulSayisi, redSayisi);

    hedefKapat(kabulHedefi);
    hedefKapat(redHedefi);
    kurallariTemizle(kurallar, kuralSayisi);
    return 0;
}