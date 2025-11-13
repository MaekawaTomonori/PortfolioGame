# PostEffect Preset Editor 再設計

## 現在の問題点

### 1. データフローの複雑さ
- Editor側で `editingKeyframeNames_` と `editingKeyframeOrder_` を保持
- Effect側でも `keyframes_` と `keyframeOrder_` を保持
- 二重管理による同期問題

### 2. 保存ロジックの分散
- `SaveKeyframesForMember()` - keyframeOrder のみ保存
- `SaveCurrentParametersToPoint()` - 個別パラメータ保存
- 2段階保存が必要で使いづらい

### 3. JSONアクセスの重複
- LoadPointParameters() でJSON読み込み
- SaveCurrentParametersToPoint() でJSON読み書き
- LoadKeyframesForMember() でJSON読み込み
- SaveKeyframesForMember() でJSON書き込み

### 4. Effect依存の設計
- LoadPointParameters() が UpdateAnimation() に依存
- t値計算の複雑さ
- Effect内部状態への間接的なアクセス

## 新設計の方針

### 原則
1. **Single Source of Truth**: JSONファイルが唯一の真実
2. **Editor = JSON Editor**: EditorはJSONを直接編集
3. **Effect = JSON Consumer**: EffectはJSONを読むだけ
4. **一括保存**: すべての変更を一つのボタンで保存

### データフロー

```
[JSON File] ←→ [Editor (In-Memory)] → [Preview] → [Effect Instance]
    ↑                                      ↓
    └──────────── Save All ────────────────┘
```

### 構造

```
Editor State:
  - editingPresetName: string
  - editingMemberIndex: int
  - editingKeyframes: map<string, json>  // 全キーフレームデータ
  - editingKeyframeOrder: vector<string> // 順序配列
  - selectedPointIndex: int

Operations:
  1. Load Member Keyframes: JSON → editingKeyframes
  2. Select Point: index → Auto preview via Effect.LoadPreset() + UpdateAnimation()
  3. Edit Point: UI → editingKeyframes[pointName]
  4. Save All: editingKeyframes → JSON (一括)
```

## 実装方針

### Phase 1: データ管理の単純化
- `editingKeyframes_` を `std::map<std::string, nlohmann::json>` として保持
- JSONの構造をそのまま保持（パラメータ情報を含む）
- キーフレーム名の一覧は `editingKeyframes_.keys()` から取得

### Phase 2: 保存の統合
- `SaveAllKeyframes()` 一つのメソッドに統合
- `editingKeyframes_` と `editingKeyframeOrder_` を同時に保存
- 保存は明示的なボタン押下でのみ実行

### Phase 3: プレビューの簡潔化
- ポイント選択時: `Effect.LoadPreset()` → `Effect.UpdateAnimation(t)`
- パラメータ編集: Editorのメモリ内JSONを編集
- プレビュー更新: メモリ内JSON → 一時ファイル → Effect.LoadPreset()

### Phase 4: UIの整理
- 「Save Changes」ボタン1つ
- 保存前に変更があるかチェック
- 保存後にEffectに再適用

## コード設計

### 新しいヘッダー構造

```cpp
class PostProcessPresetEditor {
private:
    // Keyframe editing state (simplified)
    std::string editingMemberType_;  // "Vignette"
    std::string editingMemberName_;  // "V"
    nlohmann::json editingKeyframes_;  // {"Start": {...}, "Middle": {...}}
    std::vector<std::string> editingKeyframeOrder_;  // ["Start", "Middle", "End"]
    int selectedPointIndex_ = -1;
    bool keyframesDirty_ = false;  // 未保存の変更フラグ

    // Methods
    void LoadMemberKeyframes();  // JSON → editingKeyframes_
    void SaveMemberKeyframes();  // editingKeyframes_ → JSON (All at once)
    void PreviewCurrentPoint();  // 現在のポイントをプレビュー
    void RenderPointParameters(); // editingKeyframes_[pointName] を直接編集
};
```

### 新しい操作フロー

1. **Member選択**
   ```cpp
   LoadMemberKeyframes();  // JSON → memory
   selectedPointIndex_ = 0;
   PreviewCurrentPoint();  // 最初のポイントを表示
   ```

2. **Point選択**
   ```cpp
   selectedPointIndex_ = newIndex;
   PreviewCurrentPoint();  // 自動プレビュー
   ```

3. **パラメータ編集**
   ```cpp
   // RenderPointParameters() 内
   json& pointData = editingKeyframes_[currentPointName];
   if (ImGui::DragFloat("intensity", &pointData["intensity"])) {
       keyframesDirty_ = true;
       PreviewCurrentPoint();  // リアルタイムプレビュー
   }
   ```

4. **保存**
   ```cpp
   // 一括保存ボタン
   if (ImGui::Button("Save Changes") && keyframesDirty_) {
       SaveMemberKeyframes();  // memory → JSON (all data)
       keyframesDirty_ = false;
   }
   ```

## メリット

1. **シンプル**: データは1箇所、保存は1回
2. **安全**: dirty flagで未保存を検出
3. **柔軟**: JSON構造を直接編集、エフェクト依存なし
4. **直感的**: 編集→保存の明確なフロー
5. **保守性**: 小さいメソッド、明確な責任

## 実装順序

1. `editingKeyframes_` (map<string, json>) の追加
2. `LoadMemberKeyframes()` の実装
3. `SaveMemberKeyframes()` の統合実装
4. `RenderPointParameters()` でJSON直接編集
5. `PreviewCurrentPoint()` の実装
6. 既存の重複メソッドを削除
7. dirty flag と Save Changes UI