- copying
  - 領域分割
  - from_start
  - to_start
  - startの入れ替え
    - ルートが参照する先を切り替え
  - forwardingの書き込み
  - flagsを立てる
- alloc
  - ヘッダの初期化
  - ヒープ領域の確認
    - 足りるか
    - 足りないならばGC or add_heapする
    - ヒープ領域を2分割する形にする
    - allocするのはfrom空間
- free_list
  - freeした時にリストに追加する
  - FIFO



CopyingGCで参考にできるのはRubinius
世代別GCだがminorGCでCopyingGCを採用している。
他にCopyingGCを採用している言語処理系ないしGCソフトウェアは？
- v8のminorGC

世代別GCでCopyingGCを用いるのはCopyingGCのみの利用だとメモリ効率がわるいから


rootの表現がわからないな。
rubinius(baker gc)ではroot.hppにRootオブジェクトがあり、globalsで使用される。
クラスで書かれてる。C++全然読めないんだけど。

とりあえずヒープ領域を固定で作るか。
舞台がないとなんともコーディングしづらい。

allocするにはcurrentカーソルが必要。頭から順番にallocしていく。
最初はfrom_start == currentだが、allocするとそのサイズ分だけcurrentがずれていく。

freeする == free_listに追加する

なんか間違ってる気がしなくもないけどとりあえずブロックをリンクリストにしてそれを走査する。

解放されているか否かをフラグを入れて判定するか、free_listを走査して判定するか。
フラグにしよう。swap()でfree_listは初期化される。
ヒープ走査の終了点が必要。ヒープサイズで計算しても良いかもしれないがendメンバを追加しよう。

TODO: forwarding pointer
TODO: check heap overflow
TODO: alloc from free_list

今はFL_ALLOCでアロケートを判定して、されていなければcopyしないという動作をしている。
しかし本当にやりたいのはルートから辿れるかどうかで判定すること。

TODO: prepare root
TODO: reset ref after copy