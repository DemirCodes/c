#ifndef HATA_H
#define HATA_H

#include <stddef.h>

typedef enum {
    HATA_YOK = 0,
    HATA_BELLEK,
    HATA_DOSYA,
    HATA_DISK_DOLU,
    HATA_GECERSIZ,
    HATA_ZAMAN_ASIMI
} HataKodu;

// Hata kodunu okunaklı string'e çevir
const char *hataMesaji(HataKodu kod);

// Hata log'la
void hataLogla(HataKodu kod, const char *dosya, int satir, const char *mesaj);

// Yeniden deneme ile fonksiyon çağır
// Donus: 0 basarisiz, 1 basarili
int yenidenDene(int (*fonk)(void *), void *arg, int maksDeneme, int beklemeMs);

#endif