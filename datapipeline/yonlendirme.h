#ifndef YONLENDIRME_H
#define YONLENDIRME_H

#include <stdio.h>

typedef enum {
    HEDEF_STDOUT,
    HEDEF_DOSYA,
    HEDEF_RED_LOG,
    HEDEF_HATA_LOG
} HedefTipi;//->Hangi tarzda yonlendirme yapıalcagını sectırcez (Hedef).Tanımlanmıs olan 4 adet hedef tipi var. Seçilmesi gerekir.

typedef struct {
    HedefTipi tip;  // Yukarıda olusturmus oldugumuz `HedefTipi` içerisin deki 4 adet hedef tiplemesini kullanacagımız yer.
    FILE *fp; // Hedef dosyanın adresini tutar *fp pointer ı içerisinde
    const char *ad;  // log için isim
} Hedef;  // Yonlendirme işlemini uygulayacagımız yeri belirlediğimiz bir tip

// Hedef aç (dosya hedefi için dosya açar)
Hedef *hedefAc(HedefTipi tip, const char *dosyaAdi);

// Hedefe yaz
int hedefeYaz(Hedef *h, const char *format, ...);

// Hedefi kapat
void hedefKapat(Hedef *h);

#endif