# Ui::Canvas 使い方ガイド

`Ui::Canvas` は JSON ファイルで定義した UI レイアウトを読み込み、要素の操作・アニメーション・イベント処理を行うクラスです。

---

## 基本の流れ

```
1. Canvas を unique_ptr で持つ
2. Setup() でJSONを読み込む
3. RegisterAction() でコールバックを登録する
4. SetActive() で表示/非表示を制御する
5. 毎フレーム Update / Draw は Canvas が自動で行う（UiManager 管轄）
```

---

## 1. 初期化

```cpp
ui_ = std::make_unique<Ui::Canvas>();
ui_->Setup("pause");        // Assets/Data/UI/pause.json を読み込む
ui_->SetActive(false);      // 最初は非表示にする場合
```

`Setup()` の引数はファイル名（拡張子なし）です。  
JSON は `Assets/Data/UI/<name>.json` から自動で読み込まれます。

---

## 2. JSON ファイルの構造

```json
{
    "Position": [0.0, 0.0],
    "Elements": [
        {
            "Type":     "Sprite",
            "Name":     "Background",
            "Position": [640.0, 360.0],
            "Size":     [1280.0, 720.0],
            "Texture":  "white_x16.png",
            "Color":    [0.1, 0.1, 0.1, 0.96]
        },
        {
            "Type":       "Sprite",
            "Name":       "Resume",
            "Position":   [640.0, 250.0],
            "Size":       [240.0, 96.0],
            "Texture":    "Resume.png",
            "Selectable": true,
            "Events": {
                "Execute": "Pause.Resume"
            }
        }
    ],
    "Modules": {
        "Mouse": { "Texture": "mouse-pointer.png", "Size": 32.0, "HitPadding": 16.0 }
    }
}
```

### Element 共通フィールド

| フィールド    | 型           | 説明                                       |
|------------|------------|------------------------------------------|
| `Type`     | string     | `"Sprite"` または `"Text"`                 |
| `Name`     | string     | コードから要素を取得するキー                  |
| `Position` | [x, y]     | スクリーン座標（ピクセル）                    |
| `Color`    | [r,g,b,a]  | 表示色（0.0〜1.0）                          |
| `Selectable` | bool     | カーソル/マウスで選択可能にするか             |
| `Events`   | object     | イベントキーとアクションキーのマッピング        |

### Sprite 追加フィールド

| フィールド  | 型       | 説明             |
|-----------|--------|----------------|
| `Texture` | string | テクスチャファイル名 |
| `Size`    | [w, h] | 描画サイズ（ピクセル）|

### Text 追加フィールド

| フィールド    | 型     | 説明        |
|------------|------|-----------|
| `Text`     | string | 表示文字列   |
| `FontSize` | float  | フォントサイズ |

---

## 3. アクション登録

JSON の `Events.Execute` に文字列を設定し、それをコードで関数にバインドします。

```cpp
// JSON側: "Events": { "Execute": "Pause.Resume" }

// コード側
ui_->RegisterAction("Pause.Resume", [this]() { Close(); });
ui_->RegisterAction("Pause.Retry",  [this]() { if (onRetry_) onRetry_(); });
```

### イベントキーの種類

| `EventKey`    | 発火タイミング             |
|--------------|------------------------|
| `Execute`    | 決定（Enter/Space/クリック）|
| `Hover`      | カーソルが重なったとき       |

---

## 4. 要素の取得と操作

`FindElementByName()` で名前を指定して要素を取得します。

```cpp
auto* elem = ui_->FindElementByName("current_d0");
if (!elem) return;

// テクスチャ UV クリップ（スプライトシート用）
elem->SetTextureRegion({digit * 64.f, 0.f}, {64.f, 96.f});

// テクスチャ差し替え
elem->SetTexture("numbers.png");

// 表示色の変更
elem->SetColor({1.f, 0.82f, 0.1f, 1.f});  // RGBA

// テキスト変更（TextElement）
elem->SetText("Lv 1 / 3");

// 表示/非表示
elem->SetVisible(false);

// 位置変更
elem->SetPosition({320.f, 180.f});
```

---

## 5. 全要素へのアクセス

インデックスで全要素を走査できます。

```cpp
for (size_t i = 0; i < ui_->GetElementCount(); ++i) {
    auto* elem = ui_->GetElement(i);
    if (!elem) continue;
    // 処理
}
```

---

## 6. アニメーション

`AnimFunc` は `bool(float elapsed, Vector2& posOffset, Vector4& color)` のシグネチャを持つ関数型です。  
`true` を返すとアニメーション終了です。

### Show / Hide アニメーションの設定

```cpp
// フェードイン
elem->SetShowFunc([](float elapsed, Vector2&, Vector4& color) -> bool {
    const float t = std::min(elapsed / 0.5f, 1.f);
    color.w *= t;
    return t >= 1.f;
});
elem->PlayShow();

// フェードアウト
elem->SetHideFunc([](float elapsed, Vector2&, Vector4& color) -> bool {
    const float t = std::min(elapsed / 0.5f, 1.f);
    color.w *= (1.f - t);
    return t >= 1.f;
});
elem->PlayHide();
```

### 位置オフセット付きアニメーション（KillCounter の例）

```cpp
const Vector2 startOffset = center - elem->GetPosition();

elem->SetShowFunc([startOffset, duration](float elapsed, Vector2& posOffset, Vector4& color) -> bool {
    const float t = std::min(elapsed / duration, 1.f);
    posOffset = Ease::Out::Cubic(startOffset, {0.f, 0.f}, t);
    color.w  *= t;
    return t >= 1.f;
});
elem->PlayShow();
```

`posOffset` はベース位置への **加算値** です。毎フレーム `{0, 0}` で初期化されてからアニメーション関数に渡されます。

### アニメーション再生を即完了させる

```cpp
// 空関数を設定すると PlayShow/PlayHide が即完了扱いになる
elem->SetShowFunc([](float, Vector2&, Vector4&) -> bool { return true; });
```

### Canvasレベルでアニメーション関数を登録する

JSON の `ShowAnimKey` などと紐付けたい場合は `RegisterAnimFunc()` を使います。

```cpp
ui_->RegisterAnimFunc("flash", [](float t, Vector2&, Vector4& col) -> bool {
    col.x = std::min(1.f, col.x + std::max(0.f, 1.f - t / 0.3f));
    return t >= 0.3f;
});
```

---

## 7. 表示切替

```cpp
ui_->SetActive(true);   // 表示 (ShowアニメーションをPlayShow)
ui_->SetActive(false);  // 非表示 (HideアニメーションをPlayHide)
bool open = ui_->IsActive();
```

---

## 8. イベント駆動操作（選択・実行）

```cpp
// Execute イベントを持つ要素のインデックス一覧を取得
std::vector<size_t> indices = ui_->GetIndicesWithEvent(Ui::EventKey::Execute);

// 特定インデックスのイベントを実行
ui_->ExecuteActionAt(elemIndex, Ui::EventKey::Execute);

// 要素の矩形情報を取得（カーソル位置計算に使用）
Ui::Canvas::ElementRect rect = ui_->GetElementRect(elemIndex);
// rect.position : 中心座標
// rect.size     : サイズ

// 実行時に動的にイベントを切り替える
ui_->SetElementEvent("SkillNode", Ui::EventKey::Execute, "SkillTree.Upgrade.firepower");
ui_->SetElementEvent("SkillNode", Ui::EventKey::Execute, "");  // 無効化
```

---

## 9. マウスカーソル連携

マウスが要素にホバーすると `GetCursorIndex()` でそのインデックスが返ります。  
キーボード選択とマウス選択の両立に使います。

```cpp
int canvasCursor = ui_->GetCursorIndex();
if (canvasCursor >= 0) {
    for (int i = 0; i < count; ++i) {
        if (static_cast<int>(actionIndices_[i]) == canvasCursor && i != selectedIndex_) {
            selectedIndex_ = i;
            // カーソル移動処理...
            break;
        }
    }
}
```

マウスモジュールは JSON の `Modules.Mouse` で有効化します。

---

## 10. フォーカス制御

```cpp
ui_->SetFocused(true);   // マウス/キーボード入力をこのCanvasに渡す
ui_->SetFocused(false);  // 入力を受け付けない
bool focused = ui_->HasFocus();
```

---

## 11. デバッグ UI の登録

ImGui のデバッグ画面を Canvas の Inspector に埋め込めます。

```cpp
ui_->SetCustomDebug([this]{ Debug(); });

void MyClass::Debug() {
    ImGui::DragFloat("Speed", &speed_, 0.01f);
}
```

---

## 12. 変更検知 (IsDirty)

要素の追加・削除など Canvas の構成が変わったフレームで `true` になります。  
イベント付き要素のインデックスキャッシュを更新するタイミングに使います。

```cpp
if (ui_->IsDirty()) {
    actionIndices_ = ui_->GetIndicesWithEvent(Ui::EventKey::Execute);
}
```

---

## 実装パターン早見表

| やりたいこと                      | 使う API                                              |
|--------------------------------|------------------------------------------------------|
| Canvas の初期化                  | `Setup("name")`                                      |
| 表示/非表示                      | `SetActive(true/false)`                              |
| 要素を名前で取得                   | `FindElementByName("name")`                          |
| テクスチャ UV を切り替え（数字など）  | `elem->SetTextureRegion({u, v}, {w, h})`             |
| テクスチャを差し替え               | `elem->SetTexture("file.png")`                       |
| テキストを変更                    | `elem->SetText("string")`                            |
| 色を変更                        | `elem->SetColor({r, g, b, a})`                       |
| 表示/非表示（要素単位）             | `elem->SetVisible(false)`                            |
| ボタンのコールバック登録            | `RegisterAction("Key", [](){...})`                   |
| 決定ボタン付き要素のインデックス取得  | `GetIndicesWithEvent(EventKey::Execute)`             |
| 要素を押した扱いにする             | `ExecuteActionAt(index, EventKey::Execute)`          |
| 要素の位置・サイズ取得             | `GetElementRect(index)`                              |
| Show アニメーション               | `elem->SetShowFunc(...)` + `elem->PlayShow()`        |
| Hide アニメーション               | `elem->SetHideFunc(...)` + `elem->PlayHide()`        |
| マウスホバー中の要素インデックス     | `GetCursorIndex()`                                   |
| 構成変更の検知                    | `IsDirty()`                                          |
| デバッグ UI の追加               | `SetCustomDebug([this]{ ... })`                      |
