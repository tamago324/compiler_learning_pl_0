
/* 識別子の種類 */
typedef enum {
    // 変数
    varId,
    // 関数
    funcId,
    // パラメータ
    parId,
    // 定数
    constId
} KindT;

/* 変数、パラメータ、関数のアドレスの型 */
typedef struct {
    // 入れ子の深さ
    int level;
    // level 内での相対番地 (オフセット)
    int addr;
} RelAddr;

/* ブロックの始まり (最初の変数の番地で呼ばれる？) */
void blockBegin(int firstAddr);
/* ブロックの終わりで呼び出される */
void blockEnd();

/* 現在のレベルを返す */
int bLevel();
// /* 現在の関数の引数の数を返す */
// int fPars();

/* 関数を追加 */
int enterTfunc(char *id, int v);
/* 定数を追加 */
int enterTconst(char *id, int v);
/* 変数を追加 */
int enterTvar(char *id);
/* 関数のパラメータを追加 */
int enterTpar(char *id);
/* 関数のパラメータの宣言の終わりのときに呼ばれる */
void endpar();
// /* 名前表[ti] の値の変更 (関数の先頭の番地を変更) */
// void changeV(int ti, int newVal);

/*
   名前表の 名前id の位置を返す
   未定義の場合、エラーを返す
*/
int searchT(char *id, KindT k);

/* 名前表[i]の種類を返す */
KindT kindT(int i);

/* 名前表[ti] の関数のパラメータの個数 */
int pars(int ti);
/* 名前表[tblIdx] のアドレス情報 */
RelAddr relAddr(int tblIdx);

/* そのレベルで実行時に必要な領域の数 */
int frameL();
