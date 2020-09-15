#include "getSource.h"
#include <stdio.h>

int compile() {
    Token token;

    printf("start compilation\n");

    initSource();
    while (1) {
        /* token 読み出して、出力もする */
        token = nextToken();
    }
    finalSource();

    return 1;
}
