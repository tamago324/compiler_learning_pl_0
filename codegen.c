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
/* 演算レジスタスタックの最大の長さ
   (XXX: どゆこと？？？)
*/
#define MAXREG 20

/* 命令語の情報 */
typedef struct inst {
    OpCode opCode;
    union {
        RelAddr addr;
        int value;
        Operator oppr;
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

/* 機能部、値部 の２つからなる命令語を生成 (値をセット) */
int genCodeV(OpCode op, int value) {
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.value = value;
    return cIndex;
}

/* 機能部、レベル部、オフセット の３つからなる命令語を生成 */
int genCodeT(OpCode op, int tblIdx) {
    checkMax();
    code[cIndex].opCode = op;
    code[cIndex].u.addr = relAddr(tblIdx);
    return cIndex;
}

/* 機能部、演算部 の2つからなる命令語を生成 */
int genCodeO(Operator p){
    checkMax();
    code[cIndex].opCode = opr;
    code[cIndex].u.oppr = p;
    return cIndex;
}

int genCodeR() {

    checkMax();
    code[cIndex].opCode = ret;
    code[cIndex].u.addr.level = bLevel();
    // TODO: 理解する
    /* code[cIndex].u.addr.addr = fPars(); */
    // 今は、関数呼び出しを考えないため、0にする
    code[cIndex].u.addr.addr = 0;
    return cIndex;
}

void backPatch(int i) {
    // jmp, X   の X の部分をバックパッチング
    code[i].u.value = cIndex + 1;
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
    case lit:
        printf("lit");
        flag = 1;
        break;
    case sto:
        printf("sto");
        flag = 2;
        break;
    case lod:
        printf("lod");
        flag = 2;
        break;
    case ret:
        printf("ret");
        flag = 2;
        break;
    case opr:
        printf("opr");
        flag = 3;
        break;
    case ict:
        printf("ict");
        flag = 1;
        break;
    case jmp:
        printf("jmp");
        flag = 1;
        break;
    default:
        flag = 1;
        break;
    }

    switch (flag) {
    case 1:
        // 命令形式2 (機能部、値部)
        //   -> , 値部 を表示
        printf(",%d\n", code[i].u.value);
        return;
    case 2:
        // 命令形式1 (機能部, レベル, オフセット の３つ)
        //   -> , レベル, レベル内でのオフセット の2つを表示
        printf(",%d", code[i].u.addr.level);
        printf(",%d\n", code[i].u.addr.addr);
        return;
    case 3:
        switch (code[i].u.oppr) {
        case add:
            printf(",add\n");
            break;
        case sub:
            printf(",sub\n");
            break;
        case mul:
            printf(",mul\n");
            break;
        case div:
            printf(",div\n");
            break;
        case wrt:
            printf(",wrt\n");
            break;
        case wrl:
            printf(",wrl\n");
            break;
        }
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
    // 実行時スタック (論理的には各ブロックごとに区切られていると考えられる)
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
        case lit:
            // lit, value
            // スタックの先頭に積む
            stack[top++] = i.u.value;
            break;
        case sto:
            // sto, level, addr
            // 変数の場所に、スタックの先頭のデータを格納する
            stack[display[i.u.addr.level] + i.u.addr.addr] = stack[--top];
            break;
        case lod:
            // 変数の値を取得し、格納する
            // フレーム内でのオフセットで取得できる
            stack[top++] = stack[display[i.u.addr.level] + i.u.addr.addr];
            break;
        case ret:
            // TODO: 関数呼び出しを実装するときに、display
            // の内容の回復を実装する 先頭番地の取得
            top = display[i.u.addr.level];
            // 処理を戻り番地に移す (各フレームの2番目に戻り番地が入っている)
            pc = stack[top + 1];
            break;
        case ict:
            top += i.u.value;
            if (top >= MAXMEM - MAXREG) {
                // スタックの最大を超えたら、だめ
                // XXX: MAXREG ってなに...？
                errorF("stack overflow");
            }
            break;
        case jmp:
            // ジャンプ先に制御を移す
            pc = i.u.value;
            break;
        case opr:
            switch (i.u.oppr) {
            case add:
                // top 実際に値が入っている番地より1つ先を指しているため1引く
                --top;
                stack[top - 1] += stack[top];
                // XXX: continue にする理由がわからない...
                /* continue; */
                break;
            case sub:
                --top;
                stack[top - 1] -= stack[top];
                break;
            case mul:
                --top;
                stack[top - 1] *= stack[top];
                break;
            case div:
                --top;
                stack[top - 1] /= stack[top];
                break;
            case wrt:
                // スタックの先頭を pop して表示する
                printf("%d", stack[--top]);
                break;
            case wrl:
                printf("\n");
                break;
            }
        }
    } while (pc != 0);
}
