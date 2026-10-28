#ifndef AES_H
#define AES_H

#include <stddef.h>

#define AES_ANAHTAR_UZUNLUK  32    // 256 bit
#define AES_IV_UZUNLUK       12    // 96 bit (GCM için önerilen)
#define AES_TAG_UZUNLUK      16    // 128 bit

typedef struct {
    unsigned char *sifreli;   // şifreli veri
    size_t uzunluk;           // şifreli uzunluk (orijinal ile aynı)
    unsigned char iv[AES_IV_UZUNLUK];
    unsigned char tag[AES_TAG_UZUNLUK];
} SifreliVeri;

// Anahtar üret (rastgele 32 byte)
int aesAnahtarUret(unsigned char *anahtar);

// Anahtarı hex'ten yükle
int aesAnahtarHexYukle(const char *hex, unsigned char *anahtar);

// Anahtarı hex'e çevir
char *aesAnahtarHexKodla(const unsigned char *anahtar);

// Şifrele (AES-256-GCM)
// Donus: 1 basarili, 0 hata
int aesSifrele(const unsigned char *anahtar,
               const char *acikVeri, size_t acikUzunluk,
               SifreliVeri *sonuc);

// Çöz (AES-256-GCM)
// Donus: malloc ile ayrılmış string (cagiran free etmeli), NULL hata
char *aesCoz(const unsigned char *anahtar,
             const SifreliVeri *sifreli,
             size_t *cikisUzunluk);

// SifreliVeri temizle
void aesSil(SifreliVeri *sv);

#endif