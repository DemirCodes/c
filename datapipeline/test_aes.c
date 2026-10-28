#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aes.h"

static int test_sayisi = 0, basarisiz = 0;

#define TEST(ifade) do { \
    test_sayisi++; \
    if (!(ifade)) { basarisiz++; \
        fprintf(stderr, "FAIL: %s (satir %d)\n", #ifade, __LINE__); } \
    else printf("PASS: %s\n", #ifade); \
} while (0)

void roundTrip(const char *metin, const char *etiket) {
    printf("\n=== %s ===\n", etiket);

    unsigned char anahtar[AES_ANAHTAR_UZUNLUK];
    if (!aesAnahtarUret(anahtar)) {
        printf("Anahtar uretilemedi\n");
        return;
    }

    size_t len = strlen(metin);

    // Şifrele
    SifreliVeri sv;
    if (!aesSifrele(anahtar, metin, len, &sv)) {
        printf("Sifreleme basarisiz\n");
        return;
    }

    printf("Orijinal: [%s] (%zu byte)\n", metin, len);
    printf("Sifreli:  %zu byte\n", sv.uzunluk);
    printf("IV:       ");
    for (int i = 0; i < AES_IV_UZUNLUK; i++) printf("%02x", sv.iv[i]);
    printf("\n");
    printf("Tag:      ");
    for (int i = 0; i < AES_TAG_UZUNLUK; i++) printf("%02x", sv.tag[i]);
    printf("\n");

    // Çöz
    size_t cozLen;
    char *cozulen = aesCoz(anahtar, &sv, &cozLen);

    TEST(cozulen != NULL);
    TEST(cozLen == len);
    TEST(memcmp(metin, cozulen, len) == 0);

    free(cozulen);
    aesSil(&sv);
}

int main(void) {
    printf("===== AES-256-GCM TESTLERI =====\n");

    roundTrip("merhaba dunya", "Basit");
    roundTrip("", "Bos");
    roundTrip("A", "Tek karakter");
    roundTrip("turkce karakterler: ğüşiöç", "Turkce");
    roundTrip("bu cok daha uzun bir metin, 5000 karaktere kadar test edilebilir",
              "Uzun");

    // Yanlis anahtar testi
    printf("\n=== YANLIS ANAHTAR TESTI ===\n");
    unsigned char anahtar1[AES_ANAHTAR_UZUNLUK];
    unsigned char anahtar2[AES_ANAHTAR_UZUNLUK];
    aesAnahtarUret(anahtar1);
    aesAnahtarUret(anahtar2);

    SifreliVeri sv;
    aesSifrele(anahtar1, "gizli veri", 10, &sv);

    // Yanlis anahtarla cozmeye calis
    size_t cozLen;
    char *cozulen = aesCoz(anahtar2, &sv, &cozLen);
    TEST(cozulen == NULL);   // BASARISIZ olmali!

    aesSil(&sv);

    // Bozuk veri testi
    printf("\n=== BOZUK VERI TESTI ===\n");
    unsigned char anahtar3[AES_ANAHTAR_UZUNLUK];
    aesAnahtarUret(anahtar3);

    SifreliVeri sv2;
    aesSifrele(anahtar3, "test veri", 9, &sv2);

    // Sifreli veriyi boz
    if (sv2.uzunluk > 0) sv2.sifreli[0] ^= 0xFF;

    cozulen = aesCoz(anahtar3, &sv2, &cozLen);
    TEST(cozulen == NULL);   // BASARISIZ olmali (tag mismatch)

    aesSil(&sv2);

    printf("\n==============================\n");
    printf("%d test, %d basarisiz\n", test_sayisi, basarisiz);
    return basarisiz == 0 ? 0 : 1;
}