#ifndef LOGROTASYON_H
#define LOGROTASYON_H

// Dosya boyutunu byte cinsinden al (yoksa 0)
long dosyaBoyutu(const char *dosyaAdi);

// Log dosyası maxBoyut'u aşarsa arşivle (dosya.log → dosya.log.1)
// Donus: 1 = rotasyon yapildi, 0 = yapilmadi, -1 = hata
int logRotasyon(const char *dosyaAdi, long maxBoyut);

#endif