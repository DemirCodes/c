#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "logrotasyon.h"


// logrostasyon : Bir log dosyası doldugunda veyahut zamanı doldugunda farklı bır log sayfası olusturulmasını saglıyor


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
    if (boyut < maxBoyut) return 0;   // rotasyon gerekmez

    // Arşiv dosyası adı: dosya.log → dosya.log.1
    char arsiv[512];
    snprintf(arsiv, sizeof(arsiv), "%s.1", dosyaAdi);

    // Eski arşivi sil (varsa)
    remove(arsiv);

    // Mevcut dosyayı arşive taşı
    if (rename(dosyaAdi, arsiv) != 0) {
        fprintf(stderr, "[HATA] Log rotasyonu basarisiz: %s\n", dosyaAdi);
        return -1;
    }

    fprintf(stderr, "[BILGI] Log rotasyonu: %s -> %s (%ld byte)\n",
            dosyaAdi, arsiv, boyut);
    return 1;
}