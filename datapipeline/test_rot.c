#include <stdio.h>
#include "logrotasyon.h"

int main(void) {
    // 100 byte
    FILE *fp = fopen("/tmp/bench_test/test.txt", "w");
    for (int i = 0; i < 100; i++) fputc('a', fp);
    fclose(fp);

    printf("Boyut: %ld\n", dosyaBoyutu("/tmp/bench_test/test.txt"));

    // Rotasyon 1
    int s1 = logRotasyon("/tmp/bench_test/test.txt", 50);
    printf("Rotasyon 1: %d\n", s1);

    // Yeni dosya
    fp = fopen("/tmp/bench_test/test.txt", "w");
    for (int i = 0; i < 100; i++) fputc('b', fp);
    fclose(fp);

    // Rotasyon 2
    int s2 = logRotasyon("/tmp/bench_test/test.txt", 50);
    printf("Rotasyon 2: %d\n", s2);

    return 0;
}
