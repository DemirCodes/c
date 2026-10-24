#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sifreleme.h"

static int test_sayisi = 0;
static int basarisiz = 0;

#define TEST(ifade) do { \
    test_sayisi++; \
    if (!(ifade)) { \
        basarisiz++; \
        fprintf(stderr, "FAIL: %s (satir %d)\n", #ifade, __LINE__); \
    } else { \
        printf("PASS: %s\n", #ifade); \
    } \
} while (0)

// Şifrele → hex → hex'ten çöz → şifre çöz → orijinal mi?
void roundTripTesti(const char *metin, const char *anahtar, const char *etiket) {
    printf("\n=== %s ===\n", etiket);
    printf("Orijinal: [%s]\n", metin);

    size_t len = strlen(metin);
    size_t klen = strlen(anahtar);

    // 1. Kopya al (orijinali koru)
    char *kopya = malloc(len + 1);
    if (!kopya) { fprintf(stderr, "malloc fail\n"); return; }
    strcpy(kopya, metin);

    // 2. Şifrele
    xorSifrele(kopya, len, anahtar, klen);
    printf("Sifreli byte'lar: ");
    for (size_t i = 0; i < len; i++) printf("%02x ", (unsigned char)kopya[i]);
    printf("\n");

    // 3. Hex'e çevir
    char *hex = hexKodla((unsigned char *)kopya, len);
    if (!hex) { free(kopya); return; }
    printf("Hex: %s\n", hex);

    // 4. Hex'ten byte dizisine geri çöz
    size_t cozLen = 0;
    unsigned char *byte = hexCoz(hex, &cozLen);
    if (!byte) { free(hex); free(kopya); return; }
    TEST(cozLen == len);

    // 5. XOR ile şifreyi çöz
    xorSifrele((char *)byte, cozLen, anahtar, klen);

    // 6. Orijinalle karşılaştır
    printf("Cozulen: [%s]\n", (char *)byte);
    TEST(strcmp(metin, (char *)byte) == 0);

    free(byte);
    free(hex);
    free(kopya);
}

int main(void) {
    printf("===== SIFRELEME TESTLERI =====\n");

    // 1. Temel round-trip
    roundTripTesti("merhaba dunya", "anahtar123", "Basit metin");

    // 2. Boş string
    roundTripTesti("", "anahtar", "Bos string");

    // 3. Tek karakter
    roundTripTesti("A", "k", "Tek karakter");

    // 4. Uzun metin
    roundTripTesti("bu cok daha uzun bir metin. turkce karakterler de var: ğüşiöç",
                   "gizli", "Uzun + turkce");

    // 5. Uzun anahtar
    roundTripTesti("kisa", "cok uzun bir anahtar burada", "Kisa metin, uzun anahtar");

    // 6. Sayilar + ozel karakterler
    roundTripTesti("123!@#$%^&*()", "x", "Ozel karakterler");

    // 7. Anahtar metinden uzun
    roundTripTesti("ab", "0123456789abcdef", "Anahtar metinden uzun");

    // 8. Hex kodlama/cozme dogrudan test
    printf("\n=== HEX KODLAMA TESTI ===\n");
    unsigned char veri[] = {0x00, 0x0F, 0xA5, 0xFF, 0x12};
    char *hex = hexKodla(veri, 5);
    printf("Byte: 00 0f a5 ff 12\n");
    printf("Hex : %s\n", hex);
    TEST(strcmp(hex, "000fa5ff12") == 0);

    size_t cozLen = 0;
    unsigned char *geri = hexCoz(hex, &cozLen);
    TEST(cozLen == 5);
    TEST(memcmp(veri, geri, 5) == 0);
    free(hex);
    free(geri);

    // 9. Gecersiz hex
    printf("\n=== GECERSIZ HEX TESTI ===\n");
    TEST(hexCoz("xyz", &cozLen) == NULL);     // gecersiz karakter → NULL, free gerekmez
    TEST(hexCoz("abc", &cozLen) == NULL);     // tek sayida karakter → NULL, free gerekmez

    unsigned char *bos = hexCoz("", &cozLen);
    TEST(bos != NULL);
    TEST(cozLen == 0);
    free(bos);     

    // Sonuc
    printf("\n==============================\n");
    printf("%d test, %d basarisiz\n", test_sayisi, basarisiz);
    if (basarisiz == 0) {
        printf("TUM TESTLER GECTI ✓\n");
        return 0;
    }
    printf("BASARISIZ TESTLER VAR ✗\n");
    return 1;
}