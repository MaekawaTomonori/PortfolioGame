# UI設計まとめ（Pause / SkillTree 共通思想）

## このドキュメントの目的

- 設計思想を人間・AI（Claude）双方で共有する
- Pause画面とSkillTree画面を同一設計に無理やり寄せない
- 「今できる操作だけをUIに存在させる」思想を明文化する

---

## 基本設計思想（最重要）

### UIは判断しない

- UIはロジックを持たない
- 条件判定・可否判断はすべて外部ロジックが行う
- UIは「現在可能な操作」を表示・実行するだけ

### 失敗するActionは存在させない

- 条件不足
- 既に取得済み
- 無効状態

これらはすべて **Action未登録** として表現する。

```cpp
element.SetActionKey("");
// or nullptr
```

- 押せない = Actionが存在しない
- 失敗・エラー・例外処理をUIに持たせない

---

## クラス責務整理

### UserInterface

- Elementのコンテナ
- 描画順管理
- EventSystemを保持
- UI自体は選択ロジックを持たない

```txt
UserInterface
 ├─ vector<Element>
 └─ EventSystem
```

---

### Element（最小単位）

- 表示データ（Sprite情報）
- uuid / name
- visible フラグ
- actionKey（任意）

Elementは「実行できる操作があるか」を知っているだけで、
その意味や条件は知らない。

---

### EventSystem

- `<ActionKey, ActionFunction>` のマップを持つ
- UserInterface経由で登録される
- ElementからActionKeyを受け取り、存在すれば実行

```cpp
if (eventSystem.Has(key)) {
    eventSystem.Execute(key);
}
```

---

## Pause画面の設計方針（特例）

### 特徴

- 静的UI
- 状態変化がほぼない
- 更新通知が不要

### 設計

- selectableIndices_ を保持してOK
- indexベースの選択管理OK
- Claude提案の実装をほぼそのまま採用

```txt
Pause
 ├─ selectedIndex_
 ├─ selectableIndices_
 └─ input handling
```

Pauseは「最小構成UIの完成形」として扱う。

---

## SkillTree画面の設計方針（本命）

### 特徴

- 状態が頻繁に変わる
  - ポイント増減
  - スキル解放
  - セッション切り替え
- 選択可能Elementが動的に変化する

### NG設計

- selectableIndices_ を永続保持
- index基準の選択管理
- UIが状態変化を直接判断

---

## SkillTreeでの正しいアプローチ

### selectable はキャッシュ扱い

```cpp
selectables = ui.CollectElements(
    [](Element& e){ return e.HasAction(); }
);
```

- 状態変化時に再収集
- 毎フレームでなく通知ベース

### 選択は ElementId 基準

- indexではなく uuid / id を基準にする
- 無効化されたら安全にフォールバック

---

## Action実行とUI更新フロー

```txt
[Input]
   ↓
[Element Click]
   ↓
[EventSystem Execute(ActionKey)]
   ↓
[Game / SkillTree Logic]
   ↓
[状態変化]
   ↓
[OnSkillTreeStateChanged()]
   ↓
[UI Refresh]
   - ActionKey差し替え
   - 見た目更新
```

---

## UI Refresh の考え方

- Elementの生成・破棄は行わない
- 再構築しない
- 差し替えるのは以下のみ：
  - ActionKey
  - 色・表示状態
  - visible

UIは「状態通知を受けて反映するだけ」。

---

## PauseとSkillTreeを分けて考える理由

- Pause = 静的・単純
- SkillTree = 状態駆動・動的

同じ仕組みで統一しない。
**似ていなくてよい。**

---

## 将来検討（今はやらない）

- Spriteを持たないElement
- 判定専用Element
- Component化

必要になった瞬間に導入する。

---

## 設計スタンス総括

- 最初から汎用化しない
- 動かしてから抽象化
- UIはロジックを知らない
- 押せない操作は最初から存在させない

この思想を崩さずに実装を進めること。
