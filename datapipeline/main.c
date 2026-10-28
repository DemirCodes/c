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
#include "aes.h"
#include <sys/stat.h>
#include "cli.h"

#define LOG_MAX_BOYUT  (10 * 1024 * 1024)   // 10 MB
#define MAKS_DENEME    3
#define BEKLEME_MS     100

static Hedef *g_kabulHedefi = NULL;
static Hedef *g_redHedefi = NULL;
static Kural *g_kurallar = NULL;
static int g_kuralSayisi = 0;

static void temizlik(void) {
    fprintf(stderr, "\n[BILGI] Sinyal alindi, kapatiliyor...\n");

    if (g_kabulHedefi) { hedefKapat(g_kabulHedefi); g_kabulHedefi = NULL; }
    if (g_redHedefi)   { hedefKapat(g_redHedefi);   g_redHedefi = NULL; }
    if (g_kurallar)    { kurallariTemizle(g_kurallar, g_kuralSayisi); g_kurallar = NULL; }

    fprintf(stderr, "[BILGI] Temizlik tamamlandi\n");
}

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

static int anahtarYukle(unsigned char *anahtar, const char *dosya) {
    FILE *fp = fopen(dosya, "rb");
    if (!fp) {
        fprintf(stderr, "[BILGI] Anahtar bulunamadi, uretiliyor: %s\n", dosya);
        if (!aesAnahtarUret(anahtar)) return 0;

        fp = fopen(dosya, "wb");
        if (!fp) return 0;

        fchmod(fileno(fp), 0600);

        fwrite(anahtar, 1, AES_ANAHTAR_UZUNLUK, fp);
        fclose(fp);
        return 1;
    }

    size_t okunan = fread(anahtar, 1, AES_ANAHTAR_UZUNLUK, fp);
    fclose(fp);

    if (okunan != AES_ANAHTAR_UZUNLUK) {
        fprintf(stderr, "[HATA] Anahtar dosyasi bozuk: %s\n", dosya);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int satirNo = 0;
    int kabulSayisi = 0;
    int redSayisi = 0;

    // CLI ayrıştır
    Ayarlar ayarlar;
    int cliSonuc = cliAyristir(argc, argv, &ayarlar);

    if (cliSonuc == 2) {
        // -h → yardım
        cliKullanim(argv[0]);
        return 0;
    }
    if (cliSonuc != 0) {
        // Hata
        cliKullanim(argv[0]);
        return 1;
    }

    // Girdi dosyasını aç (veya stdin)
    FILE *girdi = stdin;
    if (ayarlar.girdiDosyasi) {
        girdi = fopen(ayarlar.girdiDosyasi, "r");
        if (!girdi) {
            perror("fopen girdi");
            return 1;
        }
        if (!ayarlar.quiet)
            fprintf(stderr, "[BILGI] Girdi: %s\n", ayarlar.girdiDosyasi);
    }

    // Sinyal yakalama
    if (!sinyalBaslat(temizlik)) {
        fprintf(stderr, "[HATA] Sinyal yakalama baslatilamadi\n");
        if (girdi != stdin) fclose(girdi);
        return 1;
    }

    // Log rotasyonu
    logRotasyon(ayarlar.redDosyasi, ayarlar.logMaxBoyut);
    logRotasyon(ayarlar.kabulDosyasi, ayarlar.logMaxBoyut);

    // Kurallar
    Kural kurallar[MAX_KURAL] = {0};
    int kuralSayisi = 0;
    kurallariBaslat(kurallar, &kuralSayisi);

    g_kurallar = kurallar;
    g_kuralSayisi = kuralSayisi;

    // Hedefler
    Hedef *kabulHedefi = hedefAc(HEDEF_DOSYA, ayarlar.kabulDosyasi);
    Hedef *redHedefi   = hedefAc(HEDEF_RED_LOG, ayarlar.redDosyasi);

    g_kabulHedefi = kabulHedefi;
    g_redHedefi = redHedefi;

    if (!kabulHedefi || !redHedefi) {
        fprintf(stderr, "[HATA] Hedefler acilamadi\n");
        temizlik();
        if (girdi != stdin) fclose(girdi);
        return 1;
    }

    if (!ayarlar.quiet) {
        fprintf(stderr, "[BILGI] %d kural yuklendi\n", kuralSayisi);
        fprintf(stderr, "[BILGI] Kabul -> %s\n", ayarlar.kabulDosyasi);
        fprintf(stderr, "[BILGI] Red   -> %s\n", ayarlar.redDosyasi);
        fprintf(stderr, "[BILGI] Log max: %ld MB\n\n", ayarlar.logMaxBoyut / (1024 * 1024));
    }

    // Anahtar
    unsigned char aesAnahtar[AES_ANAHTAR_UZUNLUK];
    if (!anahtarYukle(aesAnahtar, ayarlar.anahtarDosyasi)) {
        fprintf(stderr, "[HATA] Anahtar yuklenemedi: %s\n", ayarlar.anahtarDosyasi);
        temizlik();
        if (girdi != stdin) fclose(girdi);
        return 1;
    }

    if (!ayarlar.quiet) {
        char *anahtarHex = aesAnahtarHexKodla(aesAnahtar);
        fprintf(stderr, "[BILGI] AES anahtari: %s\n", anahtarHex);
        free(anahtarHex);
    }

    if (!ayarlar.quiet)
        fprintf(stdout, "Veri gir (cikis: CTRL + D):\n");

    // Decrypt modu — kabul.txt'yi oku, çöz, ekrana yaz
    if (ayarlar.decrypt) {
        if (!ayarlar.quiet)
            fprintf(stderr, "[BILGI] Sifre cozme modu\n");

        // Anahtar yükle
        unsigned char aesAnahtar[AES_ANAHTAR_UZUNLUK];
        if (!anahtarYukle(aesAnahtar, ayarlar.anahtarDosyasi)) {
            fprintf(stderr, "[HATA] Anahtar yuklenemedi: %s\n", ayarlar.anahtarDosyasi);
            return 1;
        }

        // Kabul dosyasını aç
        FILE *kaynak = fopen(ayarlar.kabulDosyasi, "r");
        if (!kaynak) {
            perror("fopen kabul");
            return 1;
        }

        char *satir;
        int cozulenSayisi = 0;
        int hataSayisi = 0;

        while ((satir = satirOku(kaynak)) != NULL) {
            // Format: "[N] AES: iv:tag:cipher"
            // veya:  "    Uzunluk: ..."  (atla)

            // "[N] AES: " kısmını bul
            char *aesBas = strstr(satir, "AES: ");
            if (!aesBas) {
                free(satir);
                continue;   // meta satır, atla
            }

            aesBas += 5;   // "AES: " sonrası

            // "iv:tag:cipher" formatını ayrıştır
            char *ikiNokta1 = strchr(aesBas, ':');
            if (!ikiNokta1) { free(satir); continue; }
            *ikiNokta1 = '\0';
            char *ivHex = aesBas;

            char *ikiNokta2 = strchr(ikiNokta1 + 1, ':');
            if (!ikiNokta2) { free(satir); continue; }
            *ikiNokta2 = '\0';
            char *tagHex = ikiNokta1 + 1;

            char *cipherHex = ikiNokta2 + 1;

            // Hex'leri çöz
            size_t ivLen, tagLen, cipherLen;
            unsigned char *iv = hexCoz(ivHex, &ivLen);
            unsigned char *tag = hexCoz(tagHex, &tagLen);
            unsigned char *cipher = hexCoz(cipherHex, &cipherLen);

            if (!iv || !tag || !cipher ||
                ivLen != AES_IV_UZUNLUK ||
                tagLen != AES_TAG_UZUNLUK) {
                hataSayisi++;
                if (!ayarlar.quiet)
                    fprintf(stderr, "[HATA] Satir cozulemedi\n");
                free(iv); free(tag); free(cipher); free(satir);
                continue;
            }

            // SifreliVeri yapısını doldur
            SifreliVeri sv;
            sv.sifreli = cipher;
            sv.uzunluk = cipherLen;
            memcpy(sv.iv, iv, AES_IV_UZUNLUK);
            memcpy(sv.tag, tag, AES_TAG_UZUNLUK);

            // Çöz
            size_t cozLen;
            char *cozulen = aesCoz(aesAnahtar, &sv, &cozLen);

            if (cozulen) {
                cozulenSayisi++;
                printf("[%d] %s\n", cozulenSayisi, cozulen);
                free(cozulen);
            } else {
                hataSayisi++;
                if (!ayarlar.quiet)
                    fprintf(stderr, "[HATA] AES cozme basarisiz (bozuk veri veya yanlis anahtar)\n");
            }

            free(iv);
            free(tag);
            free(cipher);
            free(satir);
        }

        fclose(kaynak);

        if (!ayarlar.quiet) {
            fprintf(stderr, "\n[BILGI] Cozulen: %d | Hata: %d\n",
                    cozulenSayisi, hataSayisi);
        }

        return hataSayisi > 0 ? 1 : 0;
    }

    char *hamSatir;
    while ((hamSatir = satirOku(girdi)) != NULL) {
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

            if (!ayarlar.quiet)
                printf("[%d] REDDEDILDI\n", v->satirNo);

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

        SifreliVeri sv;
        if (!aesSifrele(aesAnahtar, v->icerik, orijinalUzunluk, &sv)) {
            hataLogla(HATA_BELLEK, "main.c", __LINE__, "AES sifreleme");
            free(acikKopya);
            free(kopya);
            kelimeListesiSil(kl);
            veriSil(v);
            continue;
        }

        char *ivHex = hexKodla(sv.iv, AES_IV_UZUNLUK);
        char *tagHex = hexKodla(sv.tag, AES_TAG_UZUNLUK);
        char *cipherHex = hexKodla(sv.sifreli, sv.uzunluk);

        if (!ivHex || !tagHex || !cipherHex) {
            hataLogla(HATA_BELLEK, "main.c", __LINE__, "hex kodlama");
            free(ivHex); free(tagHex); free(cipherHex);
            aesSil(&sv);
            free(acikKopya); free(kopya);
            kelimeListesiSil(kl); veriSil(v);
            continue;
        }

        size_t toplamLen = strlen(ivHex) + strlen(tagHex) + strlen(cipherHex) + 3;
        char *hex = malloc(toplamLen);
        if (!hex) {
            hataLogla(HATA_BELLEK, "main.c", __LINE__, "hex birlestirme");
            free(ivHex); free(tagHex); free(cipherHex);
            aesSil(&sv);
            free(acikKopya); free(kopya);
            kelimeListesiSil(kl); veriSil(v);
            continue;
        }

        snprintf(hex, toplamLen, "%s:%s:%s", ivHex, tagHex, cipherHex);

        free(ivHex);
        free(tagHex);
        free(cipherHex);

        kabulSayisi++;

        hedefeYaz(kabulHedefi,
            "[%d] AES: %s\n"
            "    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
            v->satirNo, hex, orijinalUzunluk, kl->sayi, toplam);

        if (!ayarlar.quiet) {
            printf("[%d] AES: %s\n", v->satirNo, hex);
            printf("    Uzunluk: %zu | Kelime: %d | SayiTop: %d\n",
                   orijinalUzunluk, kl->sayi, toplam);
        }

        if (ayarlar.verbose) {
            fprintf(stderr, "[SIFRE] Satir %d | Acik: [%s]\n", v->satirNo, acikKopya);
        }

        aesSil(&sv);
        free(hex);
        free(acikKopya);
        free(kopya);
        kelimeListesiSil(kl);
        veriSil(v);

        if (satirNo % 1000 == 0) {
            logRotasyon(ayarlar.redDosyasi, ayarlar.logMaxBoyut);
            logRotasyon(ayarlar.kabulDosyasi, ayarlar.logMaxBoyut);
        }
    }

    // Özet
    if (!ayarlar.quiet) {
        fprintf(stdout, "\n=== OZET ===\n");
        fprintf(stdout, "Toplam: %d | Kabul: %d | Red: %d\n",
                satirNo, kabulSayisi, redSayisi);
    }

    fprintf(stderr, "[BILGI] Toplam %d satir, %d kabul, %d red\n",
            satirNo, kabulSayisi, redSayisi);

    temizlik();
    if (girdi != stdin) fclose(girdi);
    return 0;
}