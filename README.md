# chaos CG 

## 作り方

make 一発で多分ビルドできるのでは…
zlib (圧縮ツールのライブラリ), `ng系ライブラリがインストールされていて，
`cpDefs.h` に定義された `cpp` の場所さえ適切ならば動くと思う．
`Makefile` を参照のこと．mac ならば，macports などでlibpngを入れると
多分大丈夫．なお，`cpZrw.c` は20年前ぐらいにどっかから入手．
なので今のコンパイラではたくさん警告が出る．
ウエタ的にCソースに日本語ではコメントは入れない．

## 使い方

```Shell:打点計算
% cpS in 
```

ファイル in の中身は，たとえば次の通り:

```plaintext: 
    1000 1000   /* 横・縦 ピクセル*/
    -2 2        /* ymin ymax */
    -1.75 2.25  /* xmin xmax */
    0.1 0.0     /* 初期値 */
    0.4 -1.24   /* パラメータ a and b */
    10000000		/* 写像回数 */

これで，`cpS.c` に書かれた方程式を，初期値から指定写像回数ブン回して
結果を `in.cpz` というファイルに格納します．状態空間をすなおに
縦×横でメッシュに区切ったとき，各マス目を軌道が何回打ったか，だけを記録．

次はこのファイルを `cpPutColor` に渡す

    % cpPutColor infig in.cpz 

第二引数の `.cpz` は省略可能．
`infig` は設定ファイルであり，たとえば次の通り．

    7				/* number of segments */
    100				/* gray scale of a segment */
    240.0 1.0 0.0	/* hsv data (segment + 1) */
    250.0 1.0 0.1
    260.0 0.7 1.0
    270 0.6 1.0
    280.0 0.4 1.0
    290 0.2 1.0
    300.0 0.0 1.0
    0 60 120 180 240 1000 4000 /* threshold */

最初の数値は，いくつの色区間を用意するかの指定．
ここでは N = 7 としている．
それで，N + 1 個の色を指定し，さらに N 個の閾値を決め，
各色間を頻度に合わせて着色できる．

上の例では，打点数が 0〜60 の間では，
Hue = 240, Saturation = 1.0, Value = 0 (青原色真っ暗) から
Hue = 250, Saturation = 1.0, Value = 0.1 (ほとんど青の紫, 原色, わずかに明るい) 
までを線形に補間して色付けする．
打点数が低いところで暗くしておくと立体感が出る．

あとはコードを読んでください
