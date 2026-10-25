#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "hata.h"

const char *hataMesaji(HataKodu kod) {
    switch (kod) {
        case HATA_YOK:          return "Hata yok";
        case HATA_BELLEK:       return "Bellek yetersiz";
        case HATA_DOSYA:        return "Dosya islemi basarisiz";
        case HATA_DISK_DOLU:    return "Disk dolu";
        case HATA_GECERSIZ:     return "Gecersiz veri";
        case HATA_ZAMAN_ASIMI:  return "Zaman asimi";
        default:                return "Bilinmeyen hata";
    }
}

void hataLogla(HataKodu kod, const char *dosya, int satir, const char *mesaj) {
    fprintf(stderr, "[HATA] %s:%d | %s",  // frpintf ile hata mesajını sabit bir şekilde ekranda tutmayı saglar
            dosya ? dosya : "?", satir,
            hataMesaji(kod));

    if (mesaj) fprintf(stderr, " | %s", mesaj);
    if (errno) fprintf(stderr, " | errno: %s", strerror(errno));
    fprintf(stderr, "\n");
}

int yenidenDene(int (*fonk)(void *), void *arg, int maksDeneme, int beklemeMs) {
    if (!fonk || maksDeneme <= 0) return 0;

    for (int i = 0; i < maksDeneme; i++) {
        int sonuc = fonk(arg);
        if (sonuc == 0) return 1;   // başarı

        // Son deneme değilse bekle
        if (i < maksDeneme - 1) {
            fprintf(stderr, "[YENIDEN] Deneme %d/%d basarisiz, %d ms bekleniyor\n",
                    i + 1, maksDeneme, beklemeMs);
            usleep(beklemeMs * 1000);
        }
    }

    return 0;   // tüm denemeler başarısız
}