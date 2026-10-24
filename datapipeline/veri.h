#ifndef VERI_H
#define VERI_H

#define MAX_SATIR 1024
#define MAX_KELIME 64
#define MAX_KELIME_UZUNLUK 128

typedef struct {
    char *icerik; // Satırın Kendisi (dinamik olacak)
    int uzunluk; // Satır uzunlugu 
    int satirNo; // kaçıncı satır oldugu tutulur
} Veri;

typedef struct 
{
    char **kelimeler;
    int sayi;
    int kapasite;
} KelimeListesi;


// Fonksiyon prototipleri ( şuanlık sadece alma işlemleri yapılacak)
Veri *veriAl(const char *satir, int satirNo);
void veriYazdir(const Veri *v);
void veriSil(Veri *v);
void turkceAscii(char *s);
void cokluBoslukSadelestir(char *s);

KelimeListesi *kelimeListesiOlustur(void);
void kelimeListesiSil(KelimeListesi *kl);
int kelimeEkle(KelimeListesi *kl, const char *kelime);


void trim(char *s);
void kucukHarf(char *s);
int kelimelereAyir(char *s, char kelimeler[][MAX_KELIME_UZUNLUK], int maksKelime);
int sayilariTopla(const char *s);
void ozelKarakterTemizle(char *s);


#endif


