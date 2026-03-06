# UI Event / Element / UserInterface 設計指針（Claude共有用）

本ドキュメントは、C++製自作ゲームエンジンにおける UI 設計について、
**思想・責務分離・実装方針**を Claude と共有するための設計まとめです。

---

## 全体方針（重要）

- UI は **状態駆動**である
- Element は **愚直で汎用的**に保つ
- Event は **事実通知のみ**（Bukkit / Paper の良い部分を踏襲）
- Action は **UIロジックを持たない**
- 見た目変更は **一時的オーバーライド**として扱う
- SkillTree / Pause など UI 単位でシステムを分離する

---

## 1. Event 設計

### 基本思想
- Event は「何が起きたか」だけを表す
- Event 自体は「何をするか」を知らない
- Bukkit / Paper の Event 思想を UI 用に縮小して採用する

### Event の役割
- 入力や状態変化を通知する
- 発生元 Element の情報を **読み取り専用**で渡す

```cpp
struct UIEvent {
    ElementID target;
    ElementSnapshot snapshot; // Data のコピー（読み取り専用）
};
```

## 2. Action(Listener)設計

### Actionの責務
- Eventを受け取り、処理判断を行う
- UIやゲーム状態に応じて、変更要求を生成する

```cpp
RequestChange(
    ElementId,
    Change::Color(newColor)
);
```

### ポイント
- ActionはElementの最終状態を決めない

## 3. Element設計

### 基本方針
- Elementは「ボタン専用」ではない
- 描画・判定用の最低単位
- ロジックを持たない

### Elementが持つもの

```cpp
Element {
    string uuid;
    string name;

    Data data;

    map<EventKey, ActionKey> eventAction;
}
```

### ポイント
- `<Event, ActionKey>`を持つ
- Hover/Clickなど複数のイベントに対応可能
- Element自身は「なぜ変わったか」を知らない

## 4. UserInterface設計

### 役割
- UI単位の管理(Pause/SkillTreeなど)
- EventSystemを所有
- Element群を管理する
- エディタの親元
- Json保存
  
### UserInterfaceが行う処理
- Eventの更新(判定などを行わせる)
- Eventの発火後のActionの呼び出し
- ChangeRequestの収集
- Elementへの反映
- 描画

### 特徴
- UI単位でEventSystemを分離
- 万能システム化を防ぐ

## 5. 見た目変更

### 基本方針
- Elementが「どう変わるか」を決めない
- Actionが変更要求を出す

### 例
- Hover -> 色を明るく
- Select -> サイズを少し大きく

## 6. SkillTree / 動的UIへの対応

### 方針
- Elementは再構築を行わない
- 状態変化時にはUserInterfaceに再構築要求を出す
- 「Actionが存在しない = 押しても何も起きない」ようにする

## 7. この設計で得られるメリット

- Elementの肥大化防止
- UIロジックとゲームロジックの分離
- Editor/Jsonでデータ駆動で再利用性が非常に高い
- SkillTree/Pauseなどジャンルの違うUIに両対応
- 拡張しても破壊的変更が起こりにくい

## 8. 実装時の最重要ルール

- Eventは事実通知のみ
- Elementは判断しない
- UserInterfaceが最終責任者

## 最終目的

- 賢いのはUI,愚直なのはElement
- 安全で拡張可能なUIシステム

 以上を前提として、具体実装を進める