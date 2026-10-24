#ifndef SIFRELEME_H
#define SIFRELEME_H

#include <stddef.h>

// XOR ile yerinde şifrele/çöz (aynı fonksiyon)
void xorSifrele(char *veri, size_t uzunluk, const char *anahtar, size_t anahtarUzunluk);

// Hex'e çevir (şifreli veriyi okunabilir yapmak için)
// Donus: malloc ile ayrılmış string (cagiran free etmeli)
char *hexKodla(const unsigned char *veri, size_t uzunluk);

// Hex'ten çöz
// Donus: malloc ile ayrılmış byte dizisi (cagiran free etmeli)
unsigned char *hexCoz(const char *hex, size_t *cikisUzunluk);

// Basit Base64 (opsiyonel, ileride)

#endif