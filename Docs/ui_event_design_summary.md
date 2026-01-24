# UI Event-Driven Design Summary

## 概要
本設計は、UIを**完全データ駆動**で構築し、
振る舞い（ロジック）はEventSystemに集約することを目的とする。
Elementは最小単位のデータコンテナとして振る舞い、
外部ロジック（SkillTree / Status等）と疎結合に接続される。

---

## 基本構成

### UserInterface
- UI画面単位の管理クラス
- Elementの生成・保持・順序管理
- EventSystemを1つ所有
- Action登録の唯一の窓口

### Element
- UIの最小単位
- JSONから生成・保存される

#### 保持データ
- uuid
- name
- visible
- Data
  - texture
  - position
  - size
  - color
- events : `<EventKey, ActionKey>` の配列

Elementは処理ロジックを持たず、
「どのイベントで、どのActionを呼びたいか」だけを保持する。

---

## Data
純粋な描画・配置情報。
UI以外への転用・コピー・保存を前提とする。

---

## EventSystem

### 役割
- ActionKey と Action（関数）のマッピングを保持
- Elementから発行されたActionKeyを解決し実行する

### 持つもの
- `map<ActionKey, Action>`

### 持たないもの
- UI描画ロジック
- Input判定
- Element内部状態管理

EventSystemは「振る舞いの倉庫」であり、
UI全体で共有されるコンポーネント集合として機能する。

---

## Event / Action モデル

### EventKey
- HoverEnter
- HoverExit
- Click
- Press 等

※ enum 等でコード定義（有限）

### ActionKey
- SkillTree.Unlock.Fireball
- UI.ShowTooltip
- Status.AddAttack 等

※ string / hash を想定（データ駆動・拡張可能）

---

## 初期化フロー

1. JSONからUserInterfaceを生成
2. Elementを復元
3. Elementが持つEventKeyに応じて判定対象として登録
4. コード側でActionKeyとActionをEventSystemへ登録

---

## 更新フロー

1. InputSystemがHover / Click状態を判定
2. 状態遷移を検知
3. 対象ElementがActionKeyをEventSystemへ通知
4. EventSystemがActionを実行
5. Elementが結果を反映
6. 描画

---

## 設計上の特徴

- UIとゲームロジックが完全に疎結合
- JSON保存・復元に強い
- 同一Actionを複数Elementで共有可能
- コンポーネント的だが、振る舞いは中央集権

---

## 本設計の立ち位置

本設計は以下に近い思想を持つ：
- Data-Oriented Design
- Event-Driven Architecture
- ECS未満のコンポーネントシステム

Element = Data  
EventSystem = Behavior(Component)

---

## 想定用途

- SkillTree UI
- 設定画面
- ノードベースUI
- ツール・エディタUI

