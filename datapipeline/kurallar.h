#ifndef KURALLAR_H
#define KURALLAR_H

#include <regex.h>

#define MAX_KURAL 16

typedef struct {
    char *ad;
    char *desen;
    int bayrak;
    regex_t derlenmis;
    int aktif;
} Kural;



// YENİ: Red sonucu — detaylı bilgi
typedef struct {
    int reddedildi;         // 0 = temiz, 1 = red
    char kuralAdi[128];     // hangi kural
    char desen[256];    
    char eslesenMetin[256]; // ne eşleşti
    int konum;              // veride kaçıncı karakterde
    int uzunluk;            // eşleşmenin uzunluğu
} KontrolSonucu;

int kurallariBaslat(Kural kurallar[], int *kuralSayisi);

// DEĞİŞTİ: artık KontrolSonucu döndürüyor (int yerine)
KontrolSonucu veriKontrolEt(const char *veri, Kural kurallar[], int kuralSayisi);

void kurallariTemizle(Kural kurallar[], int kuralSayisi);

#endif