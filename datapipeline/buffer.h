#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct {
    char *veri;
    size_t uzunluk;
    size_t kapasite;
} Buffer;

Buffer *bufferOlustur(size_t baslangicKapasite);
void bufferSil(Buffer *b);
int bufferEkle(Buffer *b, const char *veri, size_t uzunluk);
int bufferEkleChar(Buffer *b, char c);
char *bufferAyir(Buffer *b, char ayrac);    // ← int yerine char *
void bufferTemizle(Buffer *b);

#endif