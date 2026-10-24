#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "kurallar.h"

//Kural seti > 
static void kuralEkle(Kural kurallar[],int *sayi, const char *ad , const char *desen, int bayrak)
{
    if(*sayi >= MAX_KURAL) {
        fprintf(stderr, "UYARI: Maksimum kural sayisina ulasıldı (%d)\n",MAX_KURAL);
        return;
    }
    
    kurallar[*sayi].ad = strdup(ad);
    kurallar[*sayi].desen = strdup(desen);
    kurallar[*sayi].bayrak = bayrak ;
    kurallar[*sayi].aktif = 1 ;

    int ret = regcomp(&kurallar[*sayi].derlenmis, desen , bayrak);
    if (ret != 0){
        char hata[256];
        regerror(ret, &kurallar[*sayi].derlenmis,hata,sizeof(hata));
        fprintf(stderr, "Kural derlenmedi '%s': %s\n",ad,hata);
        kurallar[*sayi].aktif = 0;

    }
    (*sayi)++;

}

int kurallariBaslat(Kural kurallar[],int *kuralSayisi){
    *kuralSayisi = 0;

    // kural :
    kuralEkle(kurallar, kuralSayisi, "Uzunluk > 1024","^.{1025}$",REG_EXTENDED);

    // kural :
    kuralEkle(kurallar, kuralSayisi, "Tehlikeli karakter","[<>'\"`$;|\\\\]", REG_EXTENDED);

    // kural :
    kuralEkle(kurallar, kuralSayisi , "SQL: OR 1=1" , "'[[:space:]]*or[[:space:]]*'?1'?[[:space:]]*=[[:space:]]*'?1",REG_EXTENDED | REG_ICASE);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "SQL: DROP/SELECT/UNION","(drop|select|union|insert|delete|exec)[[:space:]]",REG_EXTENDED | REG_ICASE);

    // kural :
    kuralEkle(kurallar,kuralSayisi, "SQL: yorum --","--",REG_EXTENDED);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "XSS: script","<script",REG_EXTENDED | REG_ICASE);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "XSS: javascirpt:","javascript",REG_EXTENDED | REG_ICASE);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "XSS: onerror/onload","on(error|load)[[:space:]]*=",REG_EXTENDED | REG_ICASE);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "Path: ../","\\.\\./",REG_EXTENDED);
    
    // kural :
    kuralEkle(kurallar,kuralSayisi, "Komut: ; | ` $(","[;|\\$\\(]",REG_EXTENDED);

    return 0;
}



// Dönüş: 0 = temiz, 1 = reddedildi
KontrolSonucu veriKontrolEt(const char *veri, Kural kurallar[], int kuralSayisi) {
    KontrolSonucu sonuc = {0};

    for (int i = 0; i < kuralSayisi; i++) {
        if (!kurallar[i].aktif) continue;

        regmatch_t eslesme;
        int ret = regexec(&kurallar[i].derlenmis, veri, 1, &eslesme, 0);

        if (ret == 0) {
            sonuc.reddedildi = 1;
            snprintf(sonuc.kuralAdi, sizeof(sonuc.kuralAdi), "%s", kurallar[i].ad);
            snprintf(sonuc.desen, sizeof(sonuc.desen), "%s", kurallar[i].desen);  // ← YENİ

            sonuc.konum = (int)eslesme.rm_so;
            sonuc.uzunluk = (int)(eslesme.rm_eo - eslesme.rm_so);

            int kopyalanacak = sonuc.uzunluk;
            if (kopyalanacak > (int)sizeof(sonuc.eslesenMetin) - 1) {
                kopyalanacak = sizeof(sonuc.eslesenMetin) - 1;
            }
            strncpy(sonuc.eslesenMetin, veri + sonuc.konum, kopyalanacak);
            sonuc.eslesenMetin[kopyalanacak] = '\0';

            return sonuc;
        }
    }
    return sonuc;
}

void kurallariTemizle(Kural kurallar[], int kuralSayisi) {
    if (kuralSayisi > MAX_KURAL) kuralSayisi = MAX_KURAL;
    for (int i = 0; i < kuralSayisi; i++) {
        if (kurallar[i].ad) free(kurallar[i].ad);
        if (kurallar[i].desen) free(kurallar[i].desen);
        if (kurallar[i].aktif) regfree(&kurallar[i].derlenmis);
    }
}