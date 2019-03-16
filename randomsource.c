#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint32_t n;

int main(void) {

n = rand();       // #2
 
FILE * f = fopen("/dev/urandom", "rb");
fread(&n, sizeof(uint32_t), 1, f);  // #3

char hex[9];
sprintf(hex, "%08X", n);
printf("%s", hex); 
return 0;
}
