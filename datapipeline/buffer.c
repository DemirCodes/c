#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"

#define BASLANGIC_KAPASITE 64
#define BUYUME_CARPANI 2

Buffer *bufferOlustur(size_t baslangicKapasite) {
    if (baslangicKapasite == 0) baslangicKapasite = BASLANGIC_KAPASITE;

    Buffer *b = malloc(sizeof(Buffer));
    if (!b) return NULL;

    b->veri = malloc(baslangicKapasite);
    if (!b->veri) { free(b); return NULL; }

    b->uzunluk = 0;
    b->kapasite = baslangicKapasite;
    return b;
}

void bufferSil(Buffer *b) {
    if (!b) return;
    free(b->veri);
    free(b);
}

// Kapasite yetersizse büyüt
static int bufferBuyut(Buffer *b, size_t gereken) {
    if (gereken <= b->kapasite) return 1;

    size_t yeniKapasite = b->kapasite;
    while (yeniKapasite < gereken) {
        yeniKapasite *= BUYUME_CARPANI; // ***
    }

    char *yeni = realloc(b->veri, yeniKapasite);
    if (!yeni) return 0;   // realloc başarısız → eski veri korunur

    b->veri = yeni;
    b->kapasite = yeniKapasite;
    return 1;
}

int bufferEkle(Buffer *b, const char *veri, size_t uzunluk) {
    if (!b || !veri) return 0; // Eksiklik var ***
    if (!bufferBuyut(b, b->uzunluk + uzunluk)) return 0;

    memcpy(b->veri + b->uzunluk, veri, uzunluk); // memcpy = disk e bakar eger hazır da buffer ıcerısınde veri varsa o bilgieri sunar ,  memorydeki hazırda olan bilgileri getirir
    b->uzunluk += uzunluk;
    return 1;
}

int bufferEkleChar(Buffer *b, char c) { // sorgulanacak
    return bufferEkle(b, &c, 1);
}

// Ayraca kadar oku, ayrı bir string olarak döndür
// NULL = veri kalmadı
char *bufferAyir(Buffer *b, char ayrac) { // ayrac neden character tipinde ? 
    if (!b) return NULL;

    for (size_t i = 0; i < b->uzunluk; i++) {
        if (b->veri[i] == ayrac) {
            char *sonuc = malloc(i + 1); // neden ? 
            if (!sonuc) return NULL;

            memcpy(sonuc, b->veri, i);
            sonuc[i] = '\0';

            // Kalanı sola kaydır
            memmove(b->veri, b->veri + i + 1, b->uzunluk - i - 1);
            b->uzunluk -= i + 1;
            return sonuc;
        }
    }
    return NULL;   // ayraç bulunamadı
}

void bufferTemizle(Buffer *b) {
    if (b) b->uzunluk = 0;
}