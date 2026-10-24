#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "yonlendirme.h"

Hedef *hedefAc(HedefTipi tip, const char *dosyaAdi) {  // Hedef yapısı için dinamik bellek ayrılıp dolduruluıyor ve pointer döndürülüyor 
    Hedef *h = malloc(sizeof(Hedef)); // *h hedef belleğinin ayırıldığı kısım
    if (!h) return NULL;

    h->tip = tip; // 4 enumaration constant tan bir tanesi
    h->fp = NULL; // hedef dosyanın adresi 
    h->ad = NULL; // hedef dosyanın ismi 

    switch (tip) { // seçilen enumaration constant
        case HEDEF_STDOUT:  // eger işlem sadece standart ouyputsa tanımlanmıs case deki işlemler yapılacak
            h->fp = stdout; 
            h->ad = "stdout";
            break;

        case HEDEF_DOSYA:
        case HEDEF_RED_LOG:
        case HEDEF_HATA_LOG:
            if (dosyaAdi == NULL) { free(h); return NULL; } // dosya adı boş ise pointer kullanılmıyor olarak işaretlendi ve null döndü
            h->fp = fopen(dosyaAdi, "a");   // ekleme modu // stdout fn command lerınden fopen kullanılarak adresi tanımladık *h pointer ı içerisine
            if (!h->fp) {  // hedef yoksa uygulanacak islemler / yada ad eşleşmesı yoksa 
                fprintf(stderr, "[HATA] Dosya acilamadi: %s\n", dosyaAdi);
                free(h);
                return NULL;
            }
            h->ad = strdup(dosyaAdi); // hedefın adını dosyaAdından alarak pointer içerisine dosya ısmını tanımladık 
            break;
    }

    return h;
}


int hedefeYaz(Hedef *h, const char *format, ...) {
    if (!h || !h->fp || !format) return 0;
    
// va_* makroları alınan argümanlara sırayla erşişmeyi sağlayan mekanizmadır

    va_list args;// 1) argüman listesi için tutucu tanımla
    va_start(args, format); // 2) listeyi 'format'tan sonraki ilk argümandan başlat
    int sonuc = vfprintf(h->fp, format, args); // 3) listeyi vfprintf'e devret
    va_end(args); // 4) temizlik

    fflush(h->fp);   // hemen diske yaz
    return sonuc;
}

void hedefKapat(Hedef *h) {
    if (!h) return;

    if (h->fp && h->fp != stdout && h->fp != stderr) {
        fclose(h->fp);
    }
    free((void *)h->ad);
    free(h);
}