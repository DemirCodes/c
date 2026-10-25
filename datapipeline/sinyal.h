#ifndef SINYAL_H
#define SINYAL_H

#include <signal.h>

// Temizlik fonksiyonu tipi — sinyal geldiğinde çağrılır
typedef void (*TemizlikFonksiyonu)(void);

// Sinyal yakalamayı başlat
int sinyalBaslat(TemizlikFonksiyonu fonk);

// Sinyal geldi mi kontrol et (ana döngüde)
int sinyalGeldiMi(void);

// Sinyal sayacını sıfırla
void sinyalSifirla(void);

#endif