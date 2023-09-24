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