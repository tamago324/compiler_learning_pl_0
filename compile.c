#include "getSource.h"
#include <stdio.h>

int compile() {
    char c;

    printf("start compilation\n");

    initSource();
    while (1) {
        c = nextChar();
        printf("%c", c);
    }

    return 1;
}
