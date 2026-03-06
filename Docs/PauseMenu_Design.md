# ポーズメニュー実装設計

## 設計思想（ui_design_shared_with_claude.md準拠）

- **UIは判断しない** - ロジックを持たない
- **失敗するActionは存在させない** - ActionKeyが空 = 選択不可
- **Pauseは静的UI** - selectableIndices_とindexベースでOK

---

## 必要な変更

### 1. Element への追加

```cpp
// Element.hpp
class Element {
private:
    std::string actionKey_;  // 紐付けるActionKey（空なら選択不可）

public:
    void SetActionKey(const std::string& _key);
    const std::string& GetActionKey() const;
    bool HasAction() const;  // actionKey_が空でないか
};
```

### 2. UserInterface への追加

```cpp
// UserInterface.hpp
class UserInterface {
    bool active_ = true;

public:
    void SetActive(bool _active);
    bool IsActive() const;

    // 選択可能Element収集（Pause側で使用）
    std::vector<size_t> CollectSelectableIndices() const;

    // ActionKey指定で実行
    void ExecuteAction(const std::string& _actionKey);

    // 選択可能Element数
    size_t GetSelectableCount() const;

    // カーソル配置用（Elementの矩形情報）
    struct ElementRect { Vector2 position; Vector2 size; };
    ElementRect GetElementRect(size_t _elementIndex) const;
};
```

### 3. Pause 実装

```cpp
// Pause.hpp
class Pause {
    std::unique_ptr<Ui::UserInterface> ui_;
    std::unique_ptr<Sprite> cursor_;

    Input* input_ = nullptr;

    std::vector<size_t> selectableIndices_;
    int32_t selectedIndex_ = 0;

    // コールバック
    std::function<void()> onResume_;
    std::function<void()> onRetry_;
    std::function<void()> onQuit_;

public:
    void Initialize(Input* _input);
    void Update();
    void Draw();

    void Open();
    void Close();
    bool IsOpen() const;

    void SetOnResume(std::function<void()> _callback);
    void SetOnRetry(std::function<void()> _callback);
    void SetOnQuit(std::function<void()> _callback);

private:
    void HandleInput();
    void UpdateCursor();
    void Execute();
};
```

---

## データフロー

```
[キー入力] → Pause.HandleInput()
               ↓
           selectedIndex_ 更新
               ↓
           UpdateCursor()（カーソル位置更新）
               ↓
[決定入力] → Execute()
               ↓
           ui_->ExecuteAction(actionKey)
               ↓
           登録されたAction実行
```

---

## JSON形式

`Assets/Data/UI/pause.json`:
```json
{
    "Elements": [
        {
            "Name": "Background",
            "Texture": "pause_bg.png",
            "Position": [640, 360],
            "Size": [400, 300],
            "Color": [0.1, 0.1, 0.1, 0.8]
        },
        {
            "Name": "Resume",
            "Texture": "btn_resume.png",
            "Position": [640, 280],
            "Size": [200, 50],
            "Color": [1, 1, 1, 1],
            "ActionKey": "Pause.Resume"
        },
        {
            "Name": "Retry",
            "Texture": "btn_retry.png",
            "Position": [640, 360],
            "Size": [200, 50],
            "Color": [1, 1, 1, 1],
            "ActionKey": "Pause.Retry"
        },
        {
            "Name": "Quit",
            "Texture": "btn_quit.png",
            "Position": [640, 440],
            "Size": [200, 50],
            "Color": [1, 1, 1, 1],
            "ActionKey": "Pause.Quit"
        }
    ]
}
```

---

## 実装順序

1. Element: actionKey_, SetActionKey(), GetActionKey(), HasAction()
2. UserInterface: active_, CollectSelectableIndices(), ExecuteAction(), GetElementRect()
3. UserInterface.Load(): ActionKey読み込み対応
4. Pause: 全体実装
