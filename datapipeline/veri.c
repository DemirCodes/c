#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "veri.h"
#define KELIME_BASLANGIC 8




// Yeni veri oluşturur: satırı kopyala , uzunlugu hesapla işlemleri yapılır

Veri *veriAl(const char *satir, int satirNo) {
    if(satir == NULL) return NULL;

    Veri *v = malloc(sizeof(Veri)) ;
    if (v == NULL ) return NULL;

    size_t len = strlen(satir);

    // '\n' varsa at (fgets ekler)
    if (len > 0 && satir[len - 1] == '\n') {
        len--;
    }

    v->icerik = malloc(len+1); // +1 -> '\0' için

    if (v->icerik == NULL) {
        free(v);
        return NULL;
    }

    strncpy(v->icerik, satir, len);
    v->icerik[len] = '\0';
    v->uzunluk = (int)len ;
    v->satirNo = satirNo ;

    return v;
};

void veriYazdir(const Veri *v) {
    if (v == NULL) return;
    printf("[%d] (%d karakter): %s\n", v->satirNo, v->uzunluk , v->icerik);
}

void veriSil(Veri *v) {
    if (v == NULL)
    {
        return ;
    }
    free(v->icerik);
    free(v);
    
}


// Cümledeki/kelimedeki başındaki ve sonundaki boşlukları temizler
void trim(char *s) {

    if (s == NULL ) return;

    char *bas = s;
    while (*bas && isspace((unsigned char)*bas)) bas++;

    char *son = bas + strlen(bas);
    while (son > bas && isspace((unsigned char) * (son -1 ))) son--;

    size_t len = son - bas;
    memmove(s,bas,len);
    s[len] = '\0';
    
}

// Tüm harfleri kucuk harf yapar
void kucukHarf(char *s) {
    if (s == NULL ) return;
    for (; *s; s++){
            *s = (char)tolower((unsigned char)*s);
    }
}


//stringi keliemelere ayır (boşluk tab newline ayraç)
// res value : bulunan kelime sayısı
int kelimelereAyir(char *s, char kelimeler[][MAX_KELIME_UZUNLUK], int maksKelime){
    if (s == NULL || kelimeler == NULL || maksKelime <= 0 ) return 0;

    int sayac = 0;
    char *p = s;

    while (*p && sayac < maksKelime) {
        while (*p && isspace((unsigned char)*p)) p++;
        if(!*p) break;

        char *bas = p;
        while (*p && !isspace((unsigned char)*p)) p++;

        size_t len = p - bas;
        if (len >= MAX_KELIME_UZUNLUK)  len = MAX_KELIME_UZUNLUK -1;

        strncpy(kelimeler[sayac], bas , len);
        kelimeler[sayac][len] = '\0';
        sayac++;
    }

    return sayac;
}


// String deki tüm tam sayıları bulup toplama
int sayilariTopla(const char *s) {
    if (s == NULL) return 0;

    int toplam = 0;
    const char *p = s;


    while (*p) {
        if(isdigit((unsigned char)*p)) {
            int sayi = 0;
            while (isdigit((unsigned char )*p)) {
                sayi = sayi * 10 + (*p - '0');
                p++;
            }
            toplam += sayi;
        }else {
            p++;
        }

    }
    return toplam;
}


// regex unique karakterleri temizle
void ozelKarakterTemizle(char *s)
{
    if ( s == NULL ) return ;

    char *oku = s;
    char *yaz = s;

    while (*oku) {
        if (isalnum((unsigned char)*oku) || isspace((unsigned char)*oku)) {
            *yaz++ = *oku;

        }
        oku++;
    }
    *yaz = '\0';
}


// UTF-8 E göre karakter algılaması için donusum yaptırmamız lazım
void turkceAscii(char *s){
    if (s ==  NULL ) return ;

    // UTF-8 çift baytlı karakterler: 0xC3 ile başlar (Türkçe çoğunlukla)
    // 0xC4/0xC5 ile başlayanlar da var (ğ, Ğ, İ, ı)
    char *oku = s;
    char *yaz = s;

    while (*oku ){
        unsigned char c  = (unsigned char)*oku;

        if (c == 0xC3 && *(oku + 1) ) {
            unsigned char c2 = (unsigned char)*(oku + 1);
            char donusum = 0;
            switch (c2) {
                case 0xA7: donusum = 'c'; break;
                case 0x87: donusum = 'c'; break;
                case 0xBC: donusum = 'u'; break;
                case 0x9C: donusum = 'u'; break;
                case 0xB6: donusum = 'o'; break;
                case 0x96: donusum = 'o'; break;
                default: donusum = 0;break;
            }
            if (donusum){
                *yaz++= donusum;
                oku += 2;
                continue;
            }
        }
        if (c == 0xC5 && *(oku + 1)) {
            unsigned char c2 = (unsigned char)*(oku + 1);
            char donusum = 0;
            switch (c2) {
                case 0x9F: donusum = 's'; break;
                case 0x9E: donusum = 's'; break;
                case 0xB0: donusum = 'I'; break;
                case 0xB1: donusum = 'i'; break;
                default:   donusum =  0 ; break;
            }
            if (donusum) {
                *yaz++ = donusum;
                oku += 2;
                continue;
            } 
        }
        if (c == 0xC4 && *(oku + 1)) {
            unsigned char c2 = (unsigned char)*(oku + 1);
            char donusum = 0;
            switch (c2) {
                case 0x9F: donusum = 'g'; break;   
                case 0x9E: donusum = 'g'; break;   
                case 0xB1: donusum = 'i'; break;   
                default:   donusum = 0; break;
            }
            if (donusum) {
                *yaz++ = donusum;
                oku += 2;
                continue;
            }
        }
        *yaz++ = *oku++;

    }   
    *yaz = '\0';
}

void cokluBoslukSadelestir(char *s) {
    if (s == NULL) return;

    char *oku = s;
    char *yaz = s;
    int oncekiBosluk = 0;

    while (*oku) {
        if (isspace((unsigned char)*oku)) {
            if (!oncekiBosluk) {
                *yaz++ = ' ';
                oncekiBosluk = 1;
            }
        } else {
            *yaz++ = *oku;
            oncekiBosluk = 0;
        }
        oku++;
    }
    *yaz = '\0';
}




KelimeListesi *kelimeListesiOlustur(void) {
    KelimeListesi *kl = malloc(sizeof(KelimeListesi));
    if (!kl) return NULL;

    kl->kapasite = KELIME_BASLANGIC;
    kl->sayi = 0;
    kl->kelimeler = malloc(kl->kapasite * sizeof(char *));
    if (!kl->kelimeler) { free(kl); return NULL; }

    return kl;
}


void kelimeListesiSil(KelimeListesi *kl) {
    if (!kl) return;
    for (int i = 0; i < kl->sayi; i++) free(kl->kelimeler[i]);
    free(kl->kelimeler);
    free(kl);
}


int kelimeEkle(KelimeListesi *kl, const char *kelime) {
    if (!kl || !kelime) return 0;

    // Kapasite dolduysa 2 katına çıkar
    if (kl->sayi >= kl->kapasite) {
        int yeniKap = kl->kapasite * 2;
        char **yeni = realloc(kl->kelimeler, yeniKap * sizeof(char *));
        if (!yeni) return 0;

        kl->kelimeler = yeni;
        kl->kapasite = yeniKap;
    }

    // Kelimeyi kopyala
    kl->kelimeler[kl->sayi] = strdup(kelime);
    if (!kl->kelimeler[kl->sayi]) return 0;

    kl->sayi++;
    return 1;
}
