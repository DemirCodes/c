#include <stdio.h>
#include <string.h>

/*

Tip Oluşturma ;
 type variableName = value;
 
type's > int , float , chr , str



*/

char test (const char *a , const char *b , char *out, size_t outSize) {
  snprintf(out, outSize, "%s %s", a,b);
}


int main (void) {
  char sonuc[64];
  test("asda", "asdasd",sonuc , sizeof(sonuc));
  printf("sonuc: %s\n" , sonuc);

  return 0;
}
