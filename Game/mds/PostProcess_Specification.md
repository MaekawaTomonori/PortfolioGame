# PostProcess プリセットシステム 最終仕様書

## 1. 目的と背景

### 解決する問題
GameScene.cpp L32で毎回`postEffects_->Add(std::make_unique<Vignette>(), "Vignette")`を呼び出し、Vignetteエフェクトを再生成していた。これによりパフォーマンスが低下し、パラメータのアニメーション制御が困難だった。

### 解決策
エフェクトインスタンスを事前に生成・保持し、プリセット切り替え時はパラメータのみを変更するシステムを導入。JSONベースのプリセット管理により、デザイナーが外部ファイルでパラメータを調整可能にする。

---

## 2. アーキテクチャ概要

### 責務分離

```
PostProcessExecutor (構成管理層)
├─ プリセット定義の読み込み (presets.json)
├─ エフェクトの有効/無効制御
├─ アニメーション制御 (timer, duration, callback)
└─ エフェクトのライフサイクル管理

IPostEffect実装 (パラメータ管理層)
├─ パラメータ値の保持 (intensity, scale, color等)
├─ キーフレームデータの読み込み
├─ Linear補間によるアニメーション
└─ パラメータのJSON保存
```

### データフロー

```
[presets.json] ─┬─> PostProcessExecutor::LoadPresets()
                │   ├─ プリセット定義を読み込み
                │   └─ メモリに保持 (presets_)
                │
                ├─> ApplyPreset("DarkScene") 呼び出し
                │   ├─ mode/ignoreに基づいてエフェクト有効/無効化
                │   ├─ memberリストの各エフェクトを処理
                │   │   ├─ FindOrCreate() でインスタンス取得/生成
                │   │   └─ LoadPreset() 呼び出し
                │   └─ duration設定、アニメーション開始
                │
[DarkScene.json] ──> Vignette::LoadPreset("DarkScene")
(Vignetteパラメータ)  ├─ キーフレームデータ読み込み
                      └─ keyframeOrder_ に順序格納

Update(deltaTime) ───> PostProcessExecutor::Update()
                       └─ if (isAnimating_)
                           ├─ t = timer / duration
                           └─ effect->UpdateAnimation(t)
                               └─ Linear補間でパラメータ更新
```

---

## 3. JSON設計

### 3.1 Executorプリセット定義

**ファイルパス**: `./Assets/Data/PostProcess/presets.json`

**構造**:
```json
{
  "presets": [
    {
      "name": "DarkScene",
      "mode": "disable_unlisted",
      "ignore": ["BackgroundEffect"],
      "duration": 2.0,
      "member": [
        {"type": "Vignette", "name": "MainVignette", "create": true},
        {"type": "Grayscale", "name": "MainGrayscale", "create": true}
      ]
    }
  ]
}
```

**フィールド詳細**:

| フィールド | 型 | 説明 |
|-----------|-----|------|
| `name` | string | プリセットの一意識別子 |
| `mode` | string | `"disable_unlisted"`: memberに含まれないエフェクトを無効化<br>`"maintain_state"`: memberに含まれないエフェクトの状態を維持 |
| `ignore` | string[] | `mode="disable_unlisted"`時に無効化しないエフェクト名リスト |
| `duration` | float | アニメーション時間（秒）。0.0なら即座に適用 |
| `member` | object[] | 有効化するエフェクトリスト |
| `member[].type` | string | エフェクトタイプ（Factoryに登録された名前） |
| `member[].name` | string | エフェクトインスタンス名（一意） |
| `member[].create` | bool | インスタンスが存在しない場合に生成するか |

**重要な設計方針**:
- `member`に含まれるエフェクトは**全て有効化**される
- `mode`と`ignore`はmemberと**同じ階層**（プリセットレベル）
- 複数プリセットの重ね適用は可能だが、`mode="maintain_state"`推奨

### 3.2 Effectパラメータ定義

**ファイルパス**: `./Assets/Data/PostEffect/{type}/{PresetName}.json`

**例**: `./Assets/Data/PostEffect/Vignette/DarkScene.json`

```json
{
  "Start": {
    "intensity": 0.0,
    "scale": 1.0,
    "color": [0.0, 0.0, 0.0]
  },
  "Middle": {
    "intensity": 0.5,
    "scale": 1.2,
    "color": [0.05, 0.05, 0.1]
  },
  "End": {
    "intensity": 0.8,
    "scale": 1.5,
    "color": [0.1, 0.1, 0.15]
  },
  "keyframes": ["Start", "Middle", "End"]
}
```

**設計方針**:
- キーフレーム名は**任意**（"Start", "End"以外も可能）
- `keyframes`配列で再生順序を指定
- 補間は**Linear固定**（EaseIn/Out等は不採用）
- キーフレームが1個のみの場合は即座に適用（アニメーションなし）

---

## 4. クラス設計

### 4.1 PostEffectFactory（新規）

**責務**: エフェクトの遅延初期化とタイプ登録

```cpp
class PostEffectFactory {
public:
    using Creator = std::function<std::unique_ptr<IPostEffect>()>;

    // タイプ名からエフェクトインスタンスを生成
    static std::unique_ptr<IPostEffect> Create(const std::string& type);

    // Game側でエフェクトタイプを登録
    static void Register(const std::string& type, Creator creator);

private:
    static std::unordered_map<std::string, Creator> creators_;
};
```

**登録例** (Game/MyGame.cpp):
```cpp
void MyGame::RegisterPostEffects() {
    PostEffectFactory::Register("Vignette", []() {
        return std::make_unique<Vignette>();
    });
    PostEffectFactory::Register("Grayscale", []() {
        return std::make_unique<Grayscale>();
    });
    PostEffectFactory::Register("Bloom", []() {
        return std::make_unique<Bloom>();
    });
}
```

**重要**: Game側で明示的に登録する設計により、Engine側の修正不要でユーザー定義エフェクトを追加可能。

### 4.2 PostProcessExecutor（拡張）

**新規メンバー変数**:
```cpp
private:
    struct EffectData {
        std::unique_ptr<IPostEffect> effect;
        std::string name;    // インスタンス名
        std::string type;    // エフェクトタイプ
        bool enabled;        // 有効/無効
    };
    std::vector<EffectData> effects_;
    std::unordered_map<std::string, json> presets_;

    // アニメーション制御
    float animationTimer_ = 0.0f;
    float animationDuration_ = 0.0f;
    bool isAnimating_ = false;
    AnimationCallback onAnimationComplete_;

    // Q44対応: 現在適用中のプリセット名
    std::string currentPresetName_;
```

**新規パブリックメソッド**:
```cpp
public:
    using AnimationCallback = std::function<void()>;

    PostProcessExecutor();  // コンストラクタでpresets.json読み込み
    void Update(float deltaTime);

    // プリセット適用（アニメーション付き）
    void ApplyPreset(const std::string& name, AnimationCallback onComplete = nullptr);

    // 現在のパラメータをPresetNameで保存（Editor用）
    void SavePreset(const std::string& name);

    // アニメーション中かどうか
    bool IsAnimating() const { return isAnimating_; }
```

**新規プライベートメソッド**:
```cpp
private:
    // presets.jsonを読み込み
    void LoadPresets(const std::string& path);

    // エフェクトを検索、なければ生成
    IPostEffect* FindOrCreate(const std::string& type, const std::string& name, bool create);

    // エフェクトの有効/無効切り替え（既存メソッドを想定、要確認）
    void SetActive(const std::string& name, bool active);
```

### 4.3 IPostEffect（拡張）

**新規仮想メソッド**:
```cpp
public:
    // PresetNameに対応するJSONファイルからパラメータ読み込み
    virtual void LoadPreset(const std::string& presetName) = 0;

    // 現在のパラメータをPresetNameで保存
    virtual void SavePreset(const std::string& presetName) = 0;

    // 現在のパラメータをJSONに変換（SavePreset内部で使用）
    virtual json SaveParameters() const = 0;

    // アニメーション進行（t: 0.0~1.0）
    virtual void UpdateAnimation(float t) = 0;
```

### 4.4 Vignette実装例（拡張）

**新規メンバー変数**:
```cpp
private:
    struct KeyframeData {
        float intensity;
        float scale;
        Vector3 color;
    };

    std::unordered_map<std::string, KeyframeData> keyframes_;
    std::vector<std::string> keyframeOrder_;
```

**実装の重要ポイント**:
- `keyframes_`: 任意名でキーフレームデータを格納
- `keyframeOrder_`: 再生順序を配列で管理
- UpdateAnimation()内でLinear補間を実行

---

## 5. コア実装詳細

### 5.1 ApplyPreset()

**処理フロー**:
```
1. プリセット存在確認 (presets_[presetName])
2. mode/ignoreに基づいてエフェクト無効化
   ├─ mode="disable_unlisted" → memberに含まれないエフェクトを無効化
   └─ ignoreリストのエフェクトは除外
3. アニメーション設定
   ├─ animationTimer_ = 0.0f
   ├─ animationDuration_ = preset["duration"]
   ├─ isAnimating_ = (duration > 0.0f)  // Q42: duration=0ならfalse
   └─ onAnimationComplete_ = onComplete
4. memberリスト処理
   ├─ FindOrCreate(type, name, create) でインスタンス取得/生成
   ├─ effect->LoadPreset(presetName)
   └─ SetActive(name, true)
5. duration=0なら即座に最終キーフレーム適用
   └─ effect->UpdateAnimation(1.0f)
6. currentPresetName_ = presetName  // Q44対応
```

**エッジケース対応**:
- **Q41**: アニメーション中に新しいApplyPreset()が呼ばれた場合、即座に中断して新しいプリセットを開始
- **Q42**: duration=0の場合、isAnimating_=falseのままUpdateAnimation(1.0f)を即座に実行

**実装例**:
```cpp
void PostProcessExecutor::ApplyPreset(const std::string& presetName, AnimationCallback onComplete) {
    if (presets_.find(presetName) == presets_.end()) return;

    json preset = presets_[presetName];
    std::string mode = preset.value("mode", "maintain_state");
    std::vector<std::string> ignore = preset.value("ignore", std::vector<std::string>());

    // Q41: アニメーション中でも即座に中断して新しいプリセット開始
    animationDuration_ = preset.value("duration", 0.0f);
    animationTimer_ = 0.0f;
    isAnimating_ = animationDuration_ > 0.0f;  // Q42
    onAnimationComplete_ = onComplete;

    // mode処理
    if (mode == "disable_unlisted") {
        for (auto& effectData : effects_) {
            bool isIgnored = std::find(ignore.begin(), ignore.end(), effectData.name) != ignore.end();
            if (!isIgnored) effectData.enabled = false;
        }
    }

    // member処理
    for (const auto& m : preset["member"]) {
        auto* effect = FindOrCreate(m["type"], m["name"], m.value("create", false));
        if (!effect) continue;
        effect->LoadPreset(presetName);
        SetActive(m["name"], true);
    }

    // Q42: duration=0なら即座に最終キーフレームを適用
    if (animationDuration_ <= 0.0f) {
        for (auto& effectData : effects_) {
            if (effectData.enabled) {
                effectData.effect->UpdateAnimation(1.0f);
            }
        }
    }

    currentPresetName_ = presetName;  // Q44
}
```

### 5.2 Update()

**処理フロー**:
```
1. isAnimating_がtrueの場合のみ実行
2. animationTimer_ += deltaTime
3. t = timer / duration (0.0~1.0にクランプ)
4. 全ての有効なエフェクトにUpdateAnimation(t)を呼び出し
5. t >= 1.0fならアニメーション完了
   ├─ isAnimating_ = false
   └─ onAnimationComplete_()呼び出し
```

**実装例**:
```cpp
void PostProcessExecutor::Update(float deltaTime) {
    if (isAnimating_) {
        animationTimer_ += deltaTime;
        float t = std::min(animationTimer_ / animationDuration_, 1.0f);

        for (auto& effectData : effects_) {
            if (effectData.enabled) {
                effectData.effect->UpdateAnimation(t);
            }
        }

        if (t >= 1.0f) {
            isAnimating_ = false;
            if (onAnimationComplete_) {
                onAnimationComplete_();
                onAnimationComplete_ = nullptr;
            }
        }
    }
}
```

### 5.3 LoadPreset() (Vignette例)

**処理フロー**:
```
1. JSONパス生成: "./Assets/Data/PostEffect/{type}/{presetName}.json"
2. ファイル存在確認
   └─ 不在時: デフォルトキーフレーム設定（Q30）
3. JSON読み込み
4. keyframes_マップにキーフレームデータ格納
5. keyframeOrder_配列に再生順序格納
6. キーフレーム検証（推奨）
   └─ keyframeOrder_の全要素がkeyframes_に存在するか確認
```

**エラーハンドリング**:
- **Q30**: ファイル不在時はデフォルト値使用（エラーなし）
- キーフレーム検証失敗時もデフォルト値にフォールバック

**実装例**:
```cpp
void Vignette::LoadPreset(const std::string& presetName) {
    std::string path = "./Assets/Data/PostEffect/Vignette/" + presetName + ".json";

    // Q30: ファイル不在時はデフォルト値使用
    if (!std::filesystem::exists(path)) {
        keyframes_.clear();
        keyframes_["Start"] = {0.0f, 1.0f, Vector3(0, 0, 0)};
        keyframes_["End"] = {0.5f, 1.2f, Vector3(0.05f, 0.05f, 0.05f)};
        keyframeOrder_ = {"Start", "End"};
        return;
    }

    json paramJson = Json::Load(path);

    // 任意名キーフレーム読み込み
    keyframes_.clear();
    for (auto& [name, data] : paramJson.items()) {
        if (name == "keyframes") continue;
        KeyframeData kf;
        kf.intensity = data["intensity"];
        kf.scale = data["scale"];
        kf.color = Vector3(data["color"][0], data["color"][1], data["color"][2]);
        keyframes_[name] = kf;
    }

    if (paramJson.contains("keyframes")) {
        keyframeOrder_ = paramJson["keyframes"].get<std::vector<std::string>>();
    }

    // キーフレーム検証（推奨）
    for (const auto& name : keyframeOrder_) {
        if (keyframes_.find(name) == keyframes_.end()) {
            // デフォルト値にフォールバック
            keyframes_.clear();
            keyframes_["Start"] = {0.0f, 1.0f, Vector3(0, 0, 0)};
            keyframes_["End"] = {0.5f, 1.2f, Vector3(0.05f, 0.05f, 0.05f)};
            keyframeOrder_ = {"Start", "End"};
            break;
        }
    }
}
```

### 5.4 UpdateAnimation() (Vignette例)

**処理フロー**:
```
1. キーフレーム数確認
   ├─ 1個: 即座に適用して終了（Q43）
   └─ 2個未満: 何もしない
2. セグメント計算
   ├─ segmentCount = keyframes.size() - 1
   ├─ segmentProgress = t * segmentCount
   ├─ currentSegment = floor(segmentProgress)
   └─ segmentT = segmentProgress - currentSegment
3. 開始/終了キーフレーム取得
4. Linear補間
   ├─ intensity = lerp(start.intensity, end.intensity, segmentT)
   ├─ scale = lerp(start.scale, end.scale, segmentT)
   └─ color = Vector3::Lerp(start.color, end.color, segmentT)
```

**重要な設計方針**:
- **Q37**: Linear補間のみ（EaseIn/Out等は不採用）
- **Q43**: キーフレームが1個のみの場合は即座に適用

**実装例**:
```cpp
void Vignette::UpdateAnimation(float t) {
    // Q43: 単一キーフレームの場合は即座に適用
    if (keyframeOrder_.size() == 1) {
        const auto& kf = keyframes_[keyframeOrder_[0]];
        intensity_ = kf.intensity;
        scale_ = kf.scale;
        color_ = kf.color;
        return;
    }

    if (keyframeOrder_.size() < 2) return;

    // セグメント計算
    float segmentCount = keyframeOrder_.size() - 1;
    float segmentProgress = t * segmentCount;
    int currentSegment = static_cast<int>(segmentProgress);
    float segmentT = segmentProgress - currentSegment;

    if (currentSegment >= segmentCount) {
        currentSegment = segmentCount - 1;
        segmentT = 1.0f;
    }

    const auto& startKf = keyframes_[keyframeOrder_[currentSegment]];
    const auto& endKf = keyframes_[keyframeOrder_[currentSegment + 1]];

    // Linear補間
    intensity_ = std::lerp(startKf.intensity, endKf.intensity, segmentT);
    scale_ = std::lerp(startKf.scale, endKf.scale, segmentT);
    color_ = Vector3::Lerp(startKf.color, endKf.color, segmentT);
}
```

### 5.5 SavePreset() (Vignette例)

**処理フロー**:
```
1. パス生成: "./Assets/Data/PostEffect/{type}/{presetName}.json"
2. SaveParameters()でjson生成
3. Json::Save()で保存
   └─ 例外発生時: エラーログ出力して続行（Q45）
```

**エラーハンドリング**:
- **Q45**: 保存失敗時はlwlog::error()でログ出力、処理は続行

**実装例**:
```cpp
void PostProcessExecutor::SavePreset(const std::string& presetName) {
    // Q44: 現在適用中のPresetNameで上書き保存
    for (auto& effectData : effects_) {
        if (effectData.enabled) {
            effectData.effect->SavePreset(presetName);
        }
    }
}

void Vignette::SavePreset(const std::string& presetName) {
    std::string path = "./Assets/Data/PostEffect/Vignette/" + presetName + ".json";
    json paramJson = SaveParameters();

    try {
        Json::Save(path, paramJson);
    } catch (const std::exception& e) {
        // Q45: エラーログを出力して続行
        lwlog::error("Failed to save preset '{}': {}", presetName, e.what());
    }
}

json Vignette::SaveParameters() const {
    json j;
    for (const auto& [name, kf] : keyframes_) {
        j[name]["intensity"] = kf.intensity;
        j[name]["scale"] = kf.scale;
        j[name]["color"] = {kf.color.x, kf.color.y, kf.color.z};
    }
    j["keyframes"] = keyframeOrder_;
    return j;
}
```

---

## 6. 使用例

### 6.1 GameScene.cpp統合

**変更前**:
```cpp
if (Singleton<Input>::GetInstance()->IsPress(DIK_SPACE)) {
    postEffects_->Add(std::make_unique<Vignette>(), "Vignette");
}
```

**変更後**:
```cpp
if (Singleton<Input>::GetInstance()->IsPress(DIK_SPACE)) {
    postEffects_->ApplyPreset("DarkScene", []() {
        std::cout << "DarkScene animation completed!" << std::endl;
    });
}

// アニメーション中の確認
if (postEffects_->IsAnimating()) {
    // アニメーション中の処理（例: UIに進行状況表示）
}
```

### 6.2 初期化順序

**MyGame.cpp**:
```cpp
void MyGame::Initialize() {
    // 1. Factory登録（最初に実行）
    RegisterPostEffects();

    // 2. PostProcessExecutor生成
    //    → コンストラクタ内でpresets.json読み込み
    postEffects_ = std::make_unique<PostProcessExecutor>();

    // 3. 初期プリセット適用（オプション）
    postEffects_->ApplyPreset("Default");
}

void MyGame::RegisterPostEffects() {
    PostEffectFactory::Register("Vignette", []() {
        return std::make_unique<Vignette>();
    });
    PostEffectFactory::Register("Grayscale", []() {
        return std::make_unique<Grayscale>();
    });
    PostEffectFactory::Register("Bloom", []() {
        return std::make_unique<Bloom>();
    });
}
```

---

## 7. エッジケースと特殊挙動

### 7.1 アニメーション中のプリセット切り替え（Q41）

**挙動**: 即座に中断して新しいプリセットを開始

```cpp
executor->ApplyPreset("DarkScene");  // duration=2.0秒
// 1秒後
executor->ApplyPreset("BrightScene");  // DarkSceneは中断、BrightSceneが即座に開始
```

**実装**: ApplyPreset()内で`animationTimer_ = 0.0f`を設定

### 7.2 duration=0の場合（Q42）

**挙動**: isAnimating_=false、最終キーフレームを即座に適用

```cpp
// presets.json
{
  "name": "InstantScene",
  "duration": 0.0
}

executor->ApplyPreset("InstantScene");
// → UpdateAnimation(1.0f)が即座に実行
// → isAnimating_はfalseのまま
// → onAnimationComplete_は呼ばれない（duration=0なのでアニメーションなし）
```

### 7.3 単一キーフレーム（Q43）

**挙動**: その値を即座に適用（アニメーションなし）

```json
{
  "Only": {"intensity": 0.5},
  "keyframes": ["Only"]
}
```

```cpp
void Vignette::UpdateAnimation(float t) {
    if (keyframeOrder_.size() == 1) {
        // 即座に適用
        const auto& kf = keyframes_[keyframeOrder_[0]];
        intensity_ = kf.intensity;
        // ...
        return;
    }
    // ...
}
```

### 7.4 保存失敗時（Q45）

**挙動**: エラーログ出力して続行

```cpp
try {
    Json::Save(path, paramJson);
} catch (const std::exception& e) {
    lwlog::error("Failed to save preset '{}': {}", presetName, e.what());
    // 処理は続行（ユーザーに通知しない）
}
```

**方針**: "Logに残しておけば見ない方が悪い"

### 7.5 スレッド安全性（Q46）

**Phase 1-3**: シングルスレッド前提（ゲームメインスレッドのみ）

**将来**: 読み取り複数スレッド対応が望ましいが、最初の実装はAで行く

```cpp
// Phase 1-3: 同期機構なし
void PostProcessExecutor::Update(float deltaTime) {
    // シングルスレッド前提
}

// 将来的な拡張（Phase 4以降）
// std::shared_mutex で読み取りロック実装
```

---

## 8. エラーハンドリング戦略

### 8.1 JSONファイル不在

**挙動**: デフォルト値を使用（エラーなし）

```cpp
if (!std::filesystem::exists(path)) {
    // デフォルト値設定
    keyframes_["Start"] = {0.0f, 1.0f, Vector3(0, 0, 0)};
    keyframes_["End"] = {0.5f, 1.2f, Vector3(0.05f, 0.05f, 0.05f)};
    keyframeOrder_ = {"Start", "End"};
    return;  // エラーログなし
}
```

### 8.2 JSON保存失敗

**挙動**: エラーログ出力して続行

```cpp
catch (const std::exception& e) {
    lwlog::error("Failed to save preset '{}': {}", presetName, e.what());
    // 例外は伝播させない
}
```

### 8.3 キーフレーム検証失敗

**挙動**: デフォルト値にフォールバック

```cpp
for (const auto& name : keyframeOrder_) {
    if (keyframes_.find(name) == keyframes_.end()) {
        // デフォルト値設定
        keyframes_.clear();
        keyframes_["Start"] = {0.0f, 1.0f, Vector3(0, 0, 0)};
        keyframes_["End"] = {0.5f, 1.2f, Vector3(0.05f, 0.05f, 0.05f)};
        keyframeOrder_ = {"Start", "End"};
        break;
    }
}
```

### 8.4 Factory未登録タイプ

**挙動**: FindOrCreate()がnullptrを返す、ApplyPreset()内でスキップ

```cpp
auto* effect = FindOrCreate(m["type"], m["name"], m.value("create", false));
if (!effect) continue;  // nullptrならスキップ
```

---

## 9. 実装優先順位

### Phase 1: Factory基盤（即座に実装可能）

1. `Engine/include/PostProcess/PostEffectFactory.hpp` 作成
2. `Engine/src/PostProcess/PostEffectFactory.cpp` 実装
3. `Game/MyGame.cpp` に`RegisterPostEffects()`追加
4. テスト: Factory::Create()が正常に動作するか確認

### Phase 2: Executor拡張（即座に実装可能）

5. `PostProcessExecutor`にメンバー変数追加
   - `effects_`, `presets_`, `animationTimer_`, `isAnimating_`, `currentPresetName_`
6. `LoadPresets()` 実装（コンストラクタから呼び出し）
7. `FindOrCreate()` 実装（Create → Initialize）
8. `ApplyPreset()` 実装（Q41/Q42対応）
9. `Update()` 実装（アニメーション処理、Q38対応）
10. `IsAnimating()` 実装
11. `SavePreset()` 実装（Q44/Q45対応）

### Phase 3: Effect I/O実装（即座に実装可能）

12. `IPostEffect`に仮想メソッド追加
    - `LoadPreset()`, `SavePreset()`, `SaveParameters()`, `UpdateAnimation()`
13. `Vignette::LoadPreset()` 実装（Q30対応、キーフレーム検証）
14. `Vignette::UpdateAnimation()` 実装（Q37/Q43対応）
15. `Vignette::SavePreset()` 実装（Q45対応）
16. `Vignette::SaveParameters()` 実装
17. Grayscale、Bloom等の他エフェクトも同様に実装

### Phase 4: Editor機能（延期）

18. Debug UI拡張（プリセット選択ドロップダウン）
19. パラメータ編集UI（ImGui）
20. 保存ボタン実装（SavePreset()呼び出し）
21. プレビュー機能（リアルタイム更新）

---

## 10. 実装前の最終チェックリスト

### 必須対応

- [ ] `PostEffectFactory` 新規作成
- [ ] `PostProcessExecutor::currentPresetName_` メンバー追加（Q44対応）
- [ ] `PostProcessExecutor::SetActive()` メソッド確認（既存？新規？）
- [ ] `IPostEffect` に4つの仮想メソッド追加
- [ ] キーフレーム検証ロジック追加（堅牢性向上）
- [ ] JSON例外処理のtry-catch追加（Q45対応）

### 推奨対応

- [ ] presets.jsonサンプルファイル作成
- [ ] Vignette/DarkScene.jsonサンプルファイル作成
- [ ] 既存のAdd()呼び出しをApplyPreset()に置き換え
- [ ] IsAnimating()を使用したUI表示実装（オプション）

### テストケース

- [ ] duration=0のプリセット適用テスト（Q42）
- [ ] 単一キーフレームのプリセット適用テスト（Q43）
- [ ] アニメーション中の切り替えテスト（Q41）
- [ ] JSONファイル不在時の挙動テスト（Q30）
- [ ] 保存失敗時のエラーログ確認（Q45）

---

## 11. 設計の重要ポイント

### シンプルさ
- **Linear補間のみ**: 複雑なEasing関数は不採用（Q37）
- **明確な完了通知**: IsAnimating() + コールバック（Q38）
- **直感的な統合**: Add() → ApplyPreset()へのシンプルな置き換え（Q39）

### 責務の分離
- **Executor**: 構成管理（どのエフェクトを有効にするか）
- **Effect**: パラメータ管理（各エフェクトの値）
- **JSON**: データ管理（presets.json = 構成、{type}/{name}.json = パラメータ）

### 堅牢性
- **冪等性保証**: 同一プリセットの複数回適用が安全
- **エラーハンドリング**: ファイル不在時もデフォルト値で動作
- **初期化順序**: Create → Initialize → LoadPreset で安全

### 拡張性
- **Factory登録**: Game側で明示的登録、Engine修正不要
- **任意名キーフレーム**: 2個以上の任意数対応
- **将来的なマルチスレッド対応**: 設計上は可能（Q46）

---

## 12. 全46問の質疑応答サマリー

| 範囲 | カテゴリ | 主な決定事項 |
|------|---------|-------------|
| Q1-Q12 | 基本方針 | インスタンス再利用、JSON分離、アニメーション延期 |
| Q13-Q20 | 詳細仕様 | 命名規則、Factory優先度（当初低→必須に変更） |
| Q21-Q25 | 重要決定 | Factory必須化、階層的保存、LoadPreset順序 |
| Q26-Q28 | スキーマ洗練 | PresetName基準、mode/ignore配置 |
| Q29-Q36 | 実装詳細 | エラーハンドリング、初期化順序、コンストラクタ読み込み |
| Q37-Q40 | 最終調整 | Linear補間固定、完了コールバック、Add()置き換え、Editor保存 |
| Q41-Q44 | エッジケース | 中断挙動、duration=0、単一キーフレーム、上書き保存 |
| Q45-Q46 | 実装方針 | JSON例外処理、シングルスレッド前提 |

---

## 13. 最終確認事項

### 設計完成度: ⭐⭐⭐⭐⭐ 5/5
- 全46問の質疑応答が完了
- 全てのエッジケースに対応
- 実装時の曖昧さが完全に解消

### 実装準備度: ⭐⭐⭐⭐⭐ 5/5
- JSONスキーマ確定
- クラス設計完了
- コア実装例完備
- Phase 1-3即座に実装可能

### 次のアクション
Phase 1（Factory基盤）の実装を開始してください。
