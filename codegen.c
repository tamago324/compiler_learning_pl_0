#include "codegen.h"
#include "getSource.h"
#include <stdio.h>

#ifndef TBL
#define TBL
#include "table.h"
#endif

/* 目的コードの最大の数 */
#define MAXCODE 200
/* 実行時スタックの長さ */
#define MAXMEM 2000
/* ブロックの最大の深さ */
#define MAXLEVEL 5

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

void execute() {
    // 実行時スタック (論理的には各ブロックごとに区切られている)
    int stack[MAXMEM];
    // 現在見える、各レベルの先頭番地のディスプレイ
    int display[MAXLEVEL];
    int pc, top;
    // 実行する命令語
    Inst i;

    printf("start execution\n");

    // 次にスタックに入れる場所 (スタックの先頭を指す)
    top = 0;
    // 命令語のカウンタ Program Counter
    pc = 0;

    // stack[top] はレベルi の前のディスプレイの退避場所
    stack[0] = 0;
    // stack[top+1] は戻り番地
    //  call の次の番地 => 手続きから呼び出し元に戻るときに使う
    stack[1] = 0;

    // 主ブロックの先頭番地は 0 
    display[0] = 0;

    do {
        i = code[pc++];
        switch (i.opCode) {
        case sto:
            // sto, level, addr
            // 変数の場所に、スタックの先頭のデータを格納する
            stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top];
            break;
        default:
            break;
        }
    } while (pc != 0);
}
