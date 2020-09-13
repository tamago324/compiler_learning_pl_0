int openSource(char fileName[]);    /* ファイルを開く */
void closeSource();                 /* ファイルを閉じる */
void initSource();                  /* 初期化 */

char nextChar();                    /* 次のトークンを読み出し、返す */

void errorNoCheck();                /* エラーのお数をカウント、多すぎたら終わり */
void errorMessage(char *m);         /* エラーメッセージを出力 */
void errorF(char *m);               /* エラーメッセージを出力し、コンパイル終了 */
