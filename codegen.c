#include "codegen.h"
#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif

/* 目的コードの最大の数 */
#define MAXCODE 200

/* 命令語の情報 */
typedef struct inst {
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        /* Operator oppr; */
    } u;
} Inst;

/* 目的語が入る */
static Inst code[MAXCODE];

/* 目的コードのインデックスの増加とチェック */
static void checkMax();
/* 命令後の表示 */
static void printCode(int i);

/* 最後に生成した命令語のインデックス
   コードを生成するたびに加算される
*/
static int cIndex = -1;

int nextCode() { return cIndex + 1; }

/* 機能部、レベル部、オフセット の３つからなる命令語を生成 */
int genCodeT(OpCode op, int tblIdx) {
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(tblIdx);
    return cIndex;
}

/* 目的コードの表示 */
void listCode() {
    printf("\ncode\n");
    for (int i = 0; i <= cIndex; i++) {
        printf("%3d: ", i);
        printCode(i);
    }
}

void printCode(int i) {
    // 命令語の種類を表す
    int flag;
    switch (code[i].opCode) {
    case sto:
        printf("sto");
        flag = 2;
        break;
    default:
        flag = 1;
        break;
    }

    switch (flag) {
    case 2:
        // 命令形式1 (機能部, レベル, オフセット の３つ)
        //   -> , レベル, レベル内でのオフセット の2つを表示
        printf(",%d", code[i].u.addr.level);
        printf(",%d\n", code[i].u.addr.addr);
        return;
    default:
        return;
    }
}

/* 目的コードのインデックスの増加とチェック */
void checkMax() {
    if (++cIndex < MAXCODE) {
        return;
    }
    errorF("too many code");
}
