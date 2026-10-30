#include <stdio.h>
#include "logrotasyon.h"

int main(void) {
    printf("test.txt      : %ld\n", dosyaBoyutu("/tmp/bench_test/test.txt"));
    printf("test.txt.1    : %ld\n", dosyaBoyutu("/tmp/bench_test/test.txt.1"));
    printf("test.txt.2    : %ld\n", dosyaBoyutu("/tmp/bench_test/test.txt.2"));
    printf("yok.txt       : %ld\n", dosyaBoyutu("/tmp/bench_test/yok.txt"));
    return 0;
}
