#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sifreleme.h"

// XOR şifreleme/çözme — aynı işlem (simetrik)
void xorSifrele(char *veri, size_t uzunluk, const char *anahtar, size_t anahtarUzunluk) {
    if (veri == NULL || anahtar == NULL || anahtarUzunluk == 0) return;

    for (size_t i = 0; i < uzunluk; i++) {
        veri[i] ^= anahtar[i % anahtarUzunluk];   // anahtarı döngüsel uygula
    }
}

// Byte dizisini hex string'e çevir: {0xAB, 0xCD} → "abcd"
char *hexKodla(const unsigned char *veri, size_t uzunluk) {
    if (veri == NULL) return NULL;

    char *hex = malloc(uzunluk * 2 + 1);
    if (hex == NULL) return NULL;

    static const char tablo[] = "0123456789abcdef";
    for (size_t i = 0; i < uzunluk; i++) {
        hex[i * 2]     = tablo[(veri[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = tablo[veri[i] & 0x0F];
    }
    hex[uzunluk * 2] = '\0';
    return hex;
}

// Hex string'i byte dizisine çevir: "abcd" → {0xAB, 0xCD}
unsigned char *hexCoz(const char *hex, size_t *cikisUzunluk) {
    if (hex == NULL || cikisUzunluk == NULL) return NULL;

    size_t len = strlen(hex);
    if (len % 2 != 0) return NULL;   // tek sayıda karakter → geçersiz

    size_t n = len / 2;
    unsigned char *veri = malloc(n + 1);
    if (veri == NULL) return NULL;

    for (size_t i = 0; i < n; i++) {
        unsigned int yuksek, dusuk;
        char c1 = hex[i * 2];
        char c2 = hex[i * 2 + 1];

        // Hex karakter → sayı
        if (c1 >= '0' && c1 <= '9') yuksek = c1 - '0';
        else if (c1 >= 'a' && c1 <= 'f') yuksek = c1 - 'a' + 10;
        else if (c1 >= 'A' && c1 <= 'F') yuksek = c1 - 'A' + 10;
        else { free(veri); return NULL; }

        if (c2 >= '0' && c2 <= '9') dusuk = c2 - '0';
        else if (c2 >= 'a' && c2 <= 'f') dusuk = c2 - 'a' + 10;
        else if (c2 >= 'A' && c2 <= 'F') dusuk = c2 - 'A' + 10;
        else { free(veri); return NULL; }

        veri[i] = (unsigned char)((yuksek << 4) | dusuk);
    }

    veri[n] = '\0';

    *cikisUzunluk = n;
    return veri;
}