#ifndef CLI_H
#define CLI_H

typedef struct {
    const char *girdiDosyasi;      // NULL = stdin
    const char *kabulDosyasi;      // "kabul.txt"
    const char *redDosyasi;        // "red.log"
    const char *anahtarDosyasi;    // "sifre.key"
    long logMaxBoyut;              // 10 * 1024 * 1024
    int verbose;                   // 0/1
    int quiet;                     // 0/1
    int yardim;                    // 0/1 (-h)
    int decrypt;                   //
} Ayarlar;

// Varsayılan ayarlarla başlat
void cliVarsayilan(Ayarlar *a);

// Argümanları ayrıştır
// Donus: 0 basarili, 1 hata, 2 yardim istendi
int cliAyristir(int argc, char *argv[], Ayarlar *a);

// Kullanım mesajı
void cliKullanim(const char *programAdi);

#endif