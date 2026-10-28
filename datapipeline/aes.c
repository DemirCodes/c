#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include "aes.h"

int aesAnahtarUret(unsigned char *anahtar) {
    if (!anahtar) return 0;
    return RAND_bytes(anahtar, AES_ANAHTAR_UZUNLUK) == 1;
}

int aesAnahtarHexYukle(const char *hex, unsigned char *anahtar) {
    if (!hex || !anahtar) return 0;
    if (strlen(hex) != AES_ANAHTAR_UZUNLUK * 2) return 0;

    for (int i = 0; i < AES_ANAHTAR_UZUNLUK; i++) {
        unsigned int b;
        if (sscanf(hex + i * 2, "%2x", &b) != 1) return 0;
        anahtar[i] = (unsigned char)b;
    }
    return 1;
}

char *aesAnahtarHexKodla(const unsigned char *anahtar) {
    if (!anahtar) return NULL;

    char *hex = malloc(AES_ANAHTAR_UZUNLUK * 2 + 1);
    if (!hex) return NULL;

    for (int i = 0; i < AES_ANAHTAR_UZUNLUK; i++) {
        sprintf(hex + i * 2, "%02x", anahtar[i]);
    }
    hex[AES_ANAHTAR_UZUNLUK * 2] = '\0';
    return hex;
}

int aesSifrele(const unsigned char *anahtar,
               const char *acikVeri, size_t acikUzunluk,
               SifreliVeri *sonuc) {
    if (!anahtar || !acikVeri || !sonuc) return 0;

    sonuc->sifreli = NULL;
    sonuc->uzunluk = 0;

    // 1. Rastgele IV üret
    if (RAND_bytes(sonuc->iv, AES_IV_UZUNLUK) != 1) return 0;

    // 2. EVP context oluştur
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return 0;

    // 3. Şifreli veri için yer ayır (GCM: ciphertext = plaintext uzunluğu)
    sonuc->sifreli = malloc(acikUzunluk + 16);
    if (!sonuc->sifreli) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    // 4. Şifrelemeyi başlat
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, anahtar, sonuc->iv) != 1) {
        goto hata;
    }

    // 5. Şifrele
    int len;
    if (EVP_EncryptUpdate(ctx, sonuc->sifreli, &len,
                          (const unsigned char *)acikVeri, acikUzunluk) != 1) {
        goto hata;
    }
    int toplam = len;

    // 6. Son bloğu al
    if (EVP_EncryptFinal_ex(ctx, sonuc->sifreli + toplam, &len) != 1) {
        goto hata;
    }
    toplam += len;

    // 7. Auth tag al (kimlik doğrulama için)
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG,
                             AES_TAG_UZUNLUK, sonuc->tag) != 1) {
        goto hata;
    }

    sonuc->uzunluk = toplam;
    EVP_CIPHER_CTX_free(ctx);
    return 1;

hata:
    free(sonuc->sifreli);
    sonuc->sifreli = NULL;
    sonuc->uzunluk = 0;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

char *aesCoz(const unsigned char *anahtar,
             const SifreliVeri *sifreli,
             size_t *cikisUzunluk) {
    if (!anahtar || !sifreli || !sifreli->sifreli || !cikisUzunluk) return NULL;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    // Çözülmüş veri için yer
    unsigned char *cozulen = malloc(sifreli->uzunluk + 16);
    if (!cozulen) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    // Çözmeyi başlat
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, anahtar, sifreli->iv) != 1) {
        free(cozulen);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int len;
    if (EVP_DecryptUpdate(ctx, cozulen, &len,
                          sifreli->sifreli, sifreli->uzunluk) != 1) {
        free(cozulen);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    int toplam = len;

    // Auth tag set et (GCM için önemli)
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                             AES_TAG_UZUNLUK, (void *)sifreli->tag) != 1) {
        free(cozulen);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    // Son blok — burada tag doğrulaması yapılır
    if (EVP_DecryptFinal_ex(ctx, cozulen + toplam, &len) != 1) {
        // Tag doğrulaması başarısız → veri bozulmuş veya yanlış anahtar
        free(cozulen);
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }
    toplam += len;

    // Sonuna '\0' ekle (string olarak kullanılabilsin)
    cozulen[toplam] = '\0';

    *cikisUzunluk = toplam;
    EVP_CIPHER_CTX_free(ctx);
    return (char *)cozulen;
}

void aesSil(SifreliVeri *sv) {
    if (!sv) return;
    if (sv->sifreli) {
        // Belleği sıfırla (hassas veri)
        memset(sv->sifreli, 0, sv->uzunluk);
        free(sv->sifreli);
        sv->sifreli = NULL;
    }
    sv->uzunluk = 0;
}