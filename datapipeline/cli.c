#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cli.h"

void cliVarsayilan(Ayarlar *a) {
    if (!a) return;
    a->girdiDosyasi = NULL;
    a->kabulDosyasi = "kabul.txt";
    a->redDosyasi = "red.log";
    a->anahtarDosyasi = "sifre.key";
    a->logMaxBoyut = 10 * 1024 * 1024;   // 10 MB
    a->verbose = 0;
    a->quiet = 0;
    a->yardim = 0;
    a->decrypt = 0;
}

void cliKullanim(const char *programAdi) {
    printf("Kullanim: %s [SECENEKLER]\n\n", programAdi);
    printf("Secenekler:\n");
    printf("  -i DOSYA    Girdi dosyasi (yoksa stdin'den okur)\n");
    printf("  -o DOSYA    Kabul edilenlerin cikti dosyasi (varsayilan: kabul.txt)\n");
    printf("  -r DOSYA    Red log dosyasi (varsayilan: red.log)\n");
    printf("  -k DOSYA    AES anahtar dosyasi (varsayilan: sifre.key)\n");
    printf("  -s MB       Log maksimum boyut MB (varsayilan: 10)\n");
    printf("  -v          Verbose (detayli log)\n");
    printf("  -q          Quiet (sadece hatalar)\n");
    printf("  -h          Bu yardim mesajini goster\n\n");
    printf("  -d          Sifre cozme modu (kabul.txt yi cozer)\n\n");
    printf("Ornekler:\n");
    printf("  %s -i veri.txt -o temiz.txt\n", programAdi);
    printf("  cat veri.txt | %s -v\n", programAdi);
    printf("  %s -i veri.txt -s 50 -v\n", programAdi);
}

int cliAyristir(int argc, char *argv[], Ayarlar *a) {
    if (!a) return 1;

    cliVarsayilan(a);

    int opt;
    while ((opt = getopt(argc, argv, "i:o:r:k:s:vqhd")) != -1) {
        switch (opt) {
            case 'i':
                a->girdiDosyasi = optarg;
                break;
            case 'o':
                a->kabulDosyasi = optarg;
                break;
            case 'r':
                a->redDosyasi = optarg;
                break;
            case 'k':
                a->anahtarDosyasi = optarg;
                break;
            case 's': {
                long mb = atol(optarg);
                if (mb <= 0) {
                    fprintf(stderr, "[HATA] Gecersiz log boyutu: %s\n", optarg);
                    return 1;
                }
                a->logMaxBoyut = mb * 1024 * 1024;
                break;
            }
            case 'v':
                a->verbose = 1;
                break;
            case 'q':
                a->quiet = 1;
                break;
            case 'h':
                a->yardim = 1;
                return 2;
            case 'd':
                a->decrypt = 1;
                break;
            case '?':
                fprintf(stderr, "[HATA] Bilinmeyen secenek: -%c\n", optopt);
                return 1;
            default:
                return 1;
        }
    }

    return 0;
}


/*
*   Eklemem/Düzenlemem Gerekenler:
*   - secenekler kısımındaki ornek komutların hepsinin kullanımını detaylı sekılde anlatmamız lazım 
*   - Ne gibi özelliklere sahip anlaması gerekiyor ekstra bir komut ile beraber kullanabilirmi bu cli tarafını gibi sorular sorularak gelıstırılebiliritesi tartısılacak
*
*
*
*/