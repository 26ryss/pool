# pool
従来のモデルに反射の要素を加えてビリヤードのようなモデルに変更した。

## 実行方法
- 1: object[0]の初速を自身で好きなように設定する
- 2: コンパイル
- 3: 実行

## 実装した変更内容一覧
- 壁を追加
- 壁に衝突した際に反射する機能を追加
- 反射時には速度が30%減少
- 上下の壁の一部を赤色で表示して落下ゾーンとし、落下ゾーンにボールが衝突したら消える機能を追加(質量0として無限遠に飛ばす)
- stop_time(デフォルトでは400)まで実行後、盤面に残っている球の数を表示
