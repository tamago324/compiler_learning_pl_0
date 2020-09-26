
/* 命令語のコード */
typedef enum codes {
    lit,
    opr,
    lod,
    sto,
    ict,
    jmp
} OpCode;

int nextCode();

/* 命令形式1
    op: 機能部
    ti: 名前表で変数が格納されている番地 (table index)
*/
int genCodeT(OpCode op, int tblIdx);
/* 命令形式2
    op: 機能部
    value: 値部 (実際の数値 or ジャンプ先の番地)
*/
int genCodeV(OpCode op, int value);

/* i のコードのジャンプ先を現在の cIndex でバックパッチングする */
void backPatch(int i);

/* 目的コードの表示 */
void listCode();
void execute();
