# 自作ゲームエンジン UI設計まとめ

## 1. 全体方針

- UIは **描画(Sprite)** と **判定・振る舞い(Logic)** を分離する
- SpriteはUIであることを知らない
- UIは状態を持ち、見た目は状態に追従する

---

## 2. UIの最小単位（Element）

### Elementが持つもの

- Sprite（描画データ）
- Position（ローカル座標）
- Size
- Color
- Visible
- 親Element
- 子Element群

### 座標系

- Position / Size は **親基準**
- World座標は親から再帰的に算出
- Root Element は必ず存在（編集不可でも可）

---

## 3. UI管理方針

- UIReposは <名前, UI群> という形で保持する予定だったが、各UIElementのルートをUserInterfaceクラスとして独立させ、各使用箇所に持たせることにする。
- 一元的管理を放棄する。
- 各UIのボタンがJsonで管理された場合、クリック時などの動作を知ることができない懸念がある。

---

## 4. Layer構造（描画・処理順）

### 採用案

UiRepos 内に Layer を挟む

```
UiRepos
 ├ Layer(Background)
 │   └ Element群
 ├ Layer(Default)
 │   └ Element群
 └ Layer(Overlay)
     └ Element群
```

### ルール

- 描画順・処理順は Layer 順
- Layer内では親Elementのみ順序操作可能
- 子Elementは親に追従

---

## 5. コンポーネント設計

### Selectable

- フォーカス可能なUI
- 座標は持たない（owner Element から取得）
- enabled / onSelect / onDeselect

### Clickable / Hoverable（省略）

- Selectableとは独立
- 組み合わせ自由

---

## 6. キーボード操作による選択移動

### 問題点

- 単一vector順では上下左右移動が不自然
- SkillTreeのような2D配置UIでは破綻

---

## 7. 採用解決策：空間ベースナビゲーション

### 基本思想

- Selectable同士は直接つながらない
- UIManagerが空間情報から「次」を決定

### 処理概要

1. 現在選択中Selectableの中心点を取得
2. 入力方向ベクトルを定義（↑↓←→）
3. 同一Repos / Layer内のSelectableを全探索
4. 半平面判定で方向一致候補のみ抽出
5. 距離 + 角度ペナルティでスコア算出
6. 最小スコアのSelectableへフォーカス移動

### 特徴

- レイアウト変更に強い
- Grid不要
- SkillTree / 放射状UIに対応

---

## 8. 例外対応（Optional）

### 手動ナビゲーション

Selectableに上書き用リンクを用意

- overrideUp
- overrideDown
- overrideLeft
- overrideRight

自動判定を部分的に置き換え可能

---

## 9. UIエディタ（ImGui想定）

### 基本レイアウト

- Hierarchy（Repos内ツリー）
- Inspector（数値編集）
- Preview（実描画）

### 重要ポイント

- 編集は UiRepos 単位
- Inspectorでの数値調整を最優先
- 選択中Elementを枠線表示

---

## 10. 設計の要点まとめ

- UI群（UiRepos）で編集・管理
- Layerで描画順を整理
- 親子で構造を管理
- Selectableは空間推論で移動先決定
- vector順依存を捨てる

---

## 結論

この設計は：

- 自作エンジンと相性が良い
- エディタ・ランタイム双方で破綻しにくい
- SkillTreeのような複雑UIにも対応可能

実用レベルのUI基盤として十分成立する。
