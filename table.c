#include <string.h>

#ifndef TBL
#define TBL
#include "table.h"
#endif

#include "getSource.h"
// 登録可能な識別子の数
#define MAXTABLE 100
// ブロックの最大の深さ
#define MAXLEVEL 5
// 名前の最大の長さ
#define MAXNAME 31

/* 名前表のエントリーの型
   XXX: 環境のエントリ？
*/
typedef struct {
    // 名前の種類
    KindT kind;
    // 名前のつづり
    char name[MAXNAME];

    union {
        // 定数の場合
        int value; // 値

        // 変数、パラメータの場合
        RelAddr raddr; // アドレス

        // 関数の場合
        struct {
            /* RelAddr raddr; // 先頭アドレス */
            int pars; // パラメータの数
        } f;
    } u;

} TableE;

// 名前表 (0 は番兵として使う)
static TableE nameTable[MAXTABLE];
/* 現在の名前表のインデックス */
static int tIndex = 0;
/* 現在のブロックレベル */
static int level = -1;

/* index[i] には、ブロックレベル i での名前表の最後のインデックスが入る */
static int index[MAXLEVEL];
/* addr[i] には、ブロックレベル i で最後に登録した変数の番地が入る */
static int addr[MAXLEVEL];

/*
   変数の番地
    現在のレベル内で登録した変数の最後の番地を表している
*/
static int localAddr;

/* 関数のインデックス */
static int tfIndex;

/* ブロックの始まり */
void blockBegin(int firstAddr) {
    // メインのブロックのため、初期化
    if (level == -1) {
        localAddr = firstAddr;
        // 名前表は１番目からスタート (enterT で ++tIndex
        // としているため、必ず1から入る)
        tIndex = 0;
        level++;
        return;
    }

    // 入れ子の数が多すぎだから、エラーを吐く
    if (level == MAXLEVEL - 1) {
        errorF("too many nested blocks");
    }

    /* 前のブロックの情報を格納 */
    // 名前表の探索のとき、そこから下を探せるようになる
    index[level] = tIndex;
    // 次に登録する番地のベースとして使うため
    addr[level] = localAddr;
    /* そのレベル内でのオフセットになるため、初期化 */
    localAddr = firstAddr;
    /* 新しいブロックだから、１加算 */
    level++;
    return;
}

/* ブロックの終わり */
void blockEnd() {
    // １つ外のブロックの情報を復元する
    level--;
    // 名前表のインデックス
    tIndex = index[level];
    localAddr = addr[level];
}

/* 名前表に名前を登録 */
void enterT(char *id) {
    // 1から入れていく (0 は番兵として使うため)
    if (++tIndex < MAXTABLE) {
        strcpy_s(nameTable[tIndex].name, sizeof(nameTable[tIndex].name), id);
    } else {
        errorF("too many names");
    }
}

/* 関数名、先頭の番地を登録 */
int enterTfunc(char *id, int v) {
    enterT(id);
    nameTable[tIndex].kind = funcId;
    /* nameTable[tIndex].u.raddr.level = level; */
    /* // 関数の先頭の番地 */
    /* nameTable[tIndex].u.f.raddr.addr = v; */
    // パラメータ数の初期値
    nameTable[tIndex].u.f.pars = 0;
    // パラメータの管理に使うため、保持
    tfIndex = tIndex;
    return tIndex;
}

/* 変数を登録 */
int enterTvar(char *id) {
    // 名前表に登録
    enterT(id);
    // 情報を登録
    nameTable[tIndex].kind = varId;
    // store のコードを生成するとき、
    // レベルとそのレベルでのオフセットが必要なため、ここでセットする
    nameTable[tIndex].u.raddr.level = level;
    nameTable[tIndex].u.raddr.addr = localAddr++;
    return tIndex;
}

/* パラメータを登録 */
int enterTpar(char *id) {
    // 変数と変わらない...！
    //  -> パラメータは、関数本体での局所変数と同じと考えられるってこと！
    enterT(id);
    nameTable[tIndex].kind = parId;
    /* nameTable[tIndex].u.raddr.level = level; */
    /* nameTable[tIndex].u.raddr.addr は 後で、まとめて設定する */
    // 関数のパラメータ数を加算
    nameTable[tfIndex].u.f.pars++;
    return tIndex;
}

/* 定数を登録 */
int enterTconst(char *id, int v) {
    // 名前表に登録
    enterT(id);
    // 情報を登録
    nameTable[tIndex].kind = constId;
    nameTable[tIndex].u.value = v;
    return tIndex;
}

void endpar() {
    // パラメータ数
    int pars = nameTable[tfIndex].u.f.pars;
    if (pars == 0) {
        return;
    }
    /* // パラメータのアドレスの設定 */
    /* for (int i = 1; i <= pars; i++) { */
    /*     nameTable[tfIndex + i].u.raddr.addr = i - 1 - pars; */
    /* } */
}

/* #<{(| 名前表[ti] の値の変更 (関数の先頭の番地を変更) |)}># */
/* void changeV(int ti, int newVal) { */
/*     nameTable[ti].u.f.raddr.addr = newVal; */
/* } */

/* 名前id を探す */
int searchT(char *id, KindT k) {
    int i;
    i = tIndex;
    // 番兵をたてる (これにより、終わりかどうかを気にしなくていい)
    strcpy_s(nameTable[0].name, sizeof(nameTable[0].name), id);
    // 外側に探していくイメージ
    while (strcmp(id, nameTable[i].name)) {
        i--;
    }

    if (i) {
        // 0 ではなければ、みつかったということ
        return i;
    } else {
        // 0 なら、未定義とみなす
        errorType("undef");
        if (k == varId) {
            // もし、変数を探していたときは、仮登録しておく
            // これにより、その後にエラーがでなくなる
            return enterTvar(id);
        } else {
            // 見つからなかった
            return 0;
        }
    }
}

// 名前表[i]の種類を返す
KindT kindT(int i){
    return nameTable[i].kind;
}

/* 関数のパラメータの個数 */
int pars(int ti) {
    return nameTable[ti].u.f.pars;
}

/* 名前表[ti] のアドレス情報 */
RelAddr relAddr(int tblIdx) { return nameTable[tblIdx].u.raddr; }

/* そのレベルで実行時に必要な領域の数 */
int frameL() { return localAddr; }
