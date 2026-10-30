#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "logrotasyon.h"

long dosyaBoyutu(const char *dosyaAdi) {
    if (!dosyaAdi) return 0;
    struct stat st;
    if (stat(dosyaAdi, &st) == 0) {
        return (long)st.st_size;
    }
    return 0;
}

int logRotasyon(const char *dosyaAdi, long maxBoyut) {
    if (!dosyaAdi || maxBoyut <= 0) return -1;

    long boyut = dosyaBoyutu(dosyaAdi);
    if (boyut < maxBoyut) return 0;

    // B seçeneği: .1, .2, .3 ... ilk boş numarayı bul
    int numara = 1;
    char arsiv[512];
    while (1) {
        snprintf(arsiv, sizeof(arsiv), "%s.%d", dosyaAdi, numara);
        if (dosyaBoyutu(arsiv) == 0) break;
        numara++;
    }

    // rename (TEK!)
    if (rename(dosyaAdi, arsiv) != 0) {
        fprintf(stderr, "[HATA] Log rotasyonu basarisiz: %s -> %s\n",
                dosyaAdi, arsiv);
        return -1;
    }

    fprintf(stderr, "[BILGI] Log rotasyonu: %s -> %s (%ld byte)\n",
            dosyaAdi, arsiv, boyut);
    return 1;
}