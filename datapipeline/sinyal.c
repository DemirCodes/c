#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "sinyal.h"


// Voltaile : bu değişken habersiz değişebilir optimize edilmemesi gerekir
// sig_atomic_t : sinyal handler'da güvenle yazılabilecek tamsayı tipi 
static volatile sig_atomic_t sinyal_alindi = 0;
static TemizlikFonksiyonu temizlik_fonk = NULL;

static void sinyalYakala(int sig) {
    sinyal_alindi = sig;
    // Not: sinyal handler'da sadece "güvenli" işlemler yapılabilir
    // printf/malloc/free YASAK! Sadece flag set et.
}

int sinyalBaslat(TemizlikFonksiyonu fonk) {
    temizlik_fonk = fonk;

    // sigaction :  sinyal handler ını kurar (kaydeder) . 
    struct sigaction sa; // handler ın nasıl calısacagını tanımlama
    sa.sa_handler = sinyalYakala;
    sigemptyset(&sa.sa_mask); // sigemptyset : sinyal setini boşaltır
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) < 0) return 0;    // Ctrl+C
    if (sigaction(SIGTERM, &sa, NULL) < 0) return 0;   // kill
    if (sigaction(SIGHUP, &sa, NULL) < 0) return 0;    // terminal kapandı

    return 1;
}

int sinyalGeldiMi(void) {
    return sinyal_alindi;
}

void sinyalSifirla(void) {
    sinyal_alindi = 0;
}