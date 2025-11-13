# 敵AI設計: コマンドパターン × JSON駆動

## 設計思想

**核心コンセプト**: Enemyクラスを変更せず、コマンドを差し替えるだけで動作を変更可能にする

### 設計目標
- ✅ Enemyクラス本体は変更しない（開放閉鎖の原則）
- ✅ MoveCommand/AttackCommandを差し替えるだけで動作変更
- ✅ JSON設定でパラメータを完全制御
- ✅ Player/Enemy共通基底クラスでコマンドを再利用
- ✅ プレイヤーに一時適用してテスト可能

---

## アーキテクチャ

```
┌──────────────────────────────────┐
│   Entity (Player/Enemy共通基底)   │
│   ・commands_: Map<name, ICommand>│
│   ・ExecuteCommand(name)          │
└───────────┬──────────────────────┘
            │
    ┌───────┴───────┐
    │               │
┌───▼────┐     ┌───▼────┐
│ Player │     │ Enemy  │
└────────┘     └────────┘
    │               │
    └───────┬───────┘
            │ 保持
            ↓
    ┌──────────────┐
    │  ICommand[]  │ ← コマンドスロット
    └──────┬───────┘
           │
    ┌──────┴──────┬──────────┬──────────┐
    │             │          │          │
┌───▼──────┐ ┌───▼─────┐ ┌──▼──────┐ ┌──▼──────┐
│LinearMove│ │BlinkMove│ │MeleeAttk│ │RangedAtk│
└──────────┘ └─────────┘ └─────────┘ └─────────┘
```

---

## JSON設定例

### 通常の追跡敵

```json
// Assets/Data/Enemy/BasicChaser.json
{
  "name": "BasicChaser",
  "model": "enemy_basic",
  "commands": {
    "move": {
      "type": "LinearMove",
      "speed": 3.0,
      "minDistance": 0.5
    },
    "attack": {
      "type": "MeleeAttack",
      "damage": 10,
      "range": 2.0,
      "cooldown": 1.5
    }
  }
}
```

### ブリンク移動の敵

```json
// Assets/Data/Enemy/BlinkEnemy.json
{
  "name": "BlinkEnemy",
  "model": "enemy_teleport",
  "commands": {
    "move": {
      "type": "BlinkMove",           // ← コマンドだけ変更
      "blinkDistance": 5.0,
      "blinkInterval": 2.0,
      "minDistance": 2.0,
      "showTrail": true
    },
    "attack": {
      "type": "MeleeAttack",
      "damage": 15,
      "range": 2.0,
      "cooldown": 1.0
    }
  }
}
```

### プレイヤーでブリンクをテスト

```json
// Assets/Data/Player/TestBlink.json
{
  "name": "PlayerWithBlink",
  "commands": {
    "move": {
      "type": "BlinkMove",           // ← 敵と同じコマンド
      "blinkDistance": 8.0,
      "blinkInterval": 0.5
    }
  }
}
```

---

## コア実装

### 1. Entity - Player/Enemy共通基底クラス

```cpp
// Game/GameObject/Entity.hpp
#pragma once
#include "GameObject.hpp"
#include "Commands/ICommand.hpp"
#include <memory>
#include <unordered_map>

class Entity : public GameObject {
protected:
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
    Entity* target_ = nullptr;
    std::unique_ptr<Model> model_;
    float health_ = 100.0f;

public:
    void LoadFromConfig(const std::string& configPath);
    void ExecuteCommand(const std::string& commandName, float deltaTime);
    void ReplaceCommand(const std::string& name, std::unique_ptr<ICommand> cmd);

    void SetTarget(Entity* target) { target_ = target; }
    Entity* GetTarget() const { return target_; }

    std::string GetDebugInfo() const;
};
```

**ポイント**:
- `commands_`: 名前でコマンドを管理（"move", "attack"など）
- `ReplaceCommand()`: 実行中にコマンドを差し替え可能
- `target_`: ターゲットエンティティへの参照

---

### 2. ICommand - コマンド基底インターフェース

```cpp
// Game/Commands/ICommand.hpp
#pragma once
#include <string>

class Entity;

class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void Execute(Entity* executor, Entity* target, float deltaTime) = 0;
    virtual bool IsActive() const = 0;
    virtual std::string GetDebugState() const = 0;
    virtual void Reset() {}
};
```

**ポイント**:
- `Execute()`: コマンドの実行ロジック
- `GetDebugState()`: デバッグ情報表示用
- `Reset()`: 再使用時の初期化

---

### 3. LinearMoveCommand - 直線移動

```cpp
// Game/Commands/Move/LinearMoveCommand.hpp
class LinearMoveCommand : public ICommand {
public:
    struct Config {
        float speed = 3.0f;
        float minDistance = 0.5f;
    };

    LinearMoveCommand(const Config& config) : config_(config) {}

    void Execute(Entity* executor, Entity* target, float deltaTime) override {
        if (!executor || !target) return;

        Vector3 direction = target->GetPosition() - executor->GetPosition();
        float distance = direction.Length();

        if (distance <= config_.minDistance) return;

        direction = direction.Normalize();
        executor->SetPosition(
            executor->GetPosition() + direction * config_.speed * deltaTime
        );
    }

    bool IsActive() const override { return true; }
    std::string GetDebugState() const override {
        return "LinearMove [Speed: " + std::to_string(config_.speed) + "]";
    }

private:
    Config config_;
};
```

---

### 4. BlinkMoveCommand - ブリンク移動

```cpp
// Game/Commands/Move/BlinkMoveCommand.hpp
class BlinkMoveCommand : public ICommand {
public:
    struct Config {
        float blinkDistance = 5.0f;
        float blinkInterval = 2.0f;
        float minDistance = 2.0f;
        bool showTrail = true;
    };

    BlinkMoveCommand(const Config& config)
        : config_(config), timer_(0.0f), canBlink_(true) {}

    void Execute(Entity* executor, Entity* target, float deltaTime) override {
        if (!executor || !target) return;

        timer_ += deltaTime;

        // クールダウン中
        if (!canBlink_) {
            if (timer_ >= config_.blinkInterval) {
                canBlink_ = true;
                timer_ = 0.0f;
            }
            return;
        }

        // 距離チェック
        Vector3 direction = target->GetPosition() - executor->GetPosition();
        float distance = direction.Length();

        if (distance <= config_.minDistance) return;

        // ブリンク実行
        direction = direction.Normalize();
        executor->SetPosition(
            executor->GetPosition() + direction * config_.blinkDistance
        );

        canBlink_ = false;
        timer_ = 0.0f;
    }

    bool IsActive() const override { return true; }
    std::string GetDebugState() const override {
        if (!canBlink_) {
            return "Blink [Cooldown: " +
                   std::to_string(config_.blinkInterval - timer_) + "s]";
        }
        return "Blink [Ready]";
    }

    void Reset() override {
        timer_ = 0.0f;
        canBlink_ = true;
    }

private:
    Config config_;
    float timer_;
    bool canBlink_;
};
```

---

### 5. CommandFactory - JSON→コマンド生成

```cpp
// Game/Commands/CommandFactory.hpp
class CommandFactory {
public:
    static std::unique_ptr<ICommand> CreateMoveCommand(const Json::Value& json) {
        std::string type = json["type"].asString();

        if (type == "LinearMove") {
            LinearMoveCommand::Config cfg;
            cfg.speed = json.get("speed", 3.0f).asFloat();
            cfg.minDistance = json.get("minDistance", 0.5f).asFloat();
            return std::make_unique<LinearMoveCommand>(cfg);
        }
        else if (type == "BlinkMove") {
            BlinkMoveCommand::Config cfg;
            cfg.blinkDistance = json.get("blinkDistance", 5.0f).asFloat();
            cfg.blinkInterval = json.get("blinkInterval", 2.0f).asFloat();
            cfg.minDistance = json.get("minDistance", 2.0f).asFloat();
            cfg.showTrail = json.get("showTrail", true).asBool();
            return std::make_unique<BlinkMoveCommand>(cfg);
        }

        return nullptr;
    }

    static std::unique_ptr<ICommand> CreateAttackCommand(const Json::Value& json) {
        // 同様の実装
    }
};
```

---

### 6. Entity実装 - 統合

```cpp
// Game/GameObject/Entity.cpp
void Entity::LoadFromConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    Json::Value root;
    file >> root;

    // モデル読み込み
    model_ = std::make_unique<Model>();
    model_->Initialize(root["model"].asString());

    // コマンド生成
    auto& cmds = root["commands"];
    if (cmds.isMember("move")) {
        commands_["move"] = CommandFactory::CreateMoveCommand(cmds["move"]);
    }
    if (cmds.isMember("attack")) {
        commands_["attack"] = CommandFactory::CreateAttackCommand(cmds["attack"]);
    }
}

void Entity::ExecuteCommand(const std::string& name, float deltaTime) {
    auto it = commands_.find(name);
    if (it != commands_.end() && it->second) {
        it->second->Execute(this, target_, deltaTime);
    }
}

void Entity::ReplaceCommand(const std::string& name, std::unique_ptr<ICommand> cmd) {
    commands_[name] = std::move(cmd);
}

std::string Entity::GetDebugInfo() const {
    std::string info = "HP: " + std::to_string(health_) + "\n";
    for (const auto& [name, cmd] : commands_) {
        info += name + ": " + cmd->GetDebugState() + "\n";
    }
    return info;
}
```

---

## 使用例

### 基本的な使い方

```cpp
// GameScene.cpp
void GameScene::Initialize() {
    // プレイヤー
    player_ = std::make_unique<Player>();
    player_->LoadFromConfig("Assets/Data/Player/Default.json");

    // 通常の敵
    auto chaser = std::make_unique<Enemy>();
    chaser->LoadFromConfig("Assets/Data/Enemy/BasicChaser.json");
    chaser->SetTarget(player_.get());
    enemies_.push_back(std::move(chaser));

    // ブリンク敵
    auto blinker = std::make_unique<Enemy>();
    blinker->LoadFromConfig("Assets/Data/Enemy/BlinkEnemy.json");
    blinker->SetTarget(player_.get());
    enemies_.push_back(std::move(blinker));
}

void GameScene::Update(float deltaTime) {
    for (auto& enemy : enemies_) {
        enemy->ExecuteCommand("move", deltaTime);
        enemy->ExecuteCommand("attack", deltaTime);
    }
}
```

### 🧪 ブリンクのテスト（プレイヤーに適用）

```cpp
void GameScene::TestBlinkOnPlayer() {
    // プレイヤーの移動をブリンクに差し替え
    BlinkMoveCommand::Config testCfg;
    testCfg.blinkDistance = 8.0f;
    testCfg.blinkInterval = 0.5f;

    player_->ReplaceCommand("move",
        std::make_unique<BlinkMoveCommand>(testCfg)
    );

    // これでプレイヤーがブリンク移動になる
    // 操作感を確認して問題なければ敵に適用
}
```

### 実行中のコマンド差し替え

```cpp
// ボスの第2形態
void Boss::EnterPhase2() {
    BlinkMoveCommand::Config cfg;
    cfg.blinkDistance = 10.0f;
    cfg.blinkInterval = 3.0f;

    ReplaceCommand("move", std::make_unique<BlinkMoveCommand>(cfg));
}
```

### デバッグ表示

```cpp
void GameScene::DrawDebugUI() {
    ImGui::Begin("Enemy Debug");
    for (size_t i = 0; i < enemies_.size(); ++i) {
        ImGui::Text("Enemy %zu:", i);
        ImGui::TextWrapped("%s", enemies_[i]->GetDebugInfo().c_str());
    }
    ImGui::End();
}

// 出力例:
// Enemy 0:
//   move: LinearMove [Speed: 3.0]
//   attack: Melee [Ready]
//
// Enemy 1:
//   move: Blink [Cooldown: 1.2s]
//   attack: Melee [Ready]
```

---

## 利点まとめ

| 利点 | 説明 |
|------|------|
| **Enemyクラス不変** | 新コマンド追加時もEnemy.cppは変更不要 |
| **即座にテスト** | プレイヤーに適用して操作感確認可能 |
| **JSON完全制御** | プログラマー以外でも調整可能 |
| **再利用性** | Player/Enemy/NPCで同じコマンド使用 |
| **動的変更** | 実行中にコマンド差し替え可能 |
| **デバッグ容易** | 現在の状態が一目瞭然 |

---

## 設計上の懸念点と検討事項

### 1. コマンド間の依存関係

**問題**: 「移動してから攻撃」のような順序依存の動作をどう表現するか

**現状の設計**:
```cpp
enemy->ExecuteCommand("move", deltaTime);
enemy->ExecuteCommand("attack", deltaTime);
```
この順序は固定されており、動的な変更が困難

**考えられる解決策**:
- A) コマンドキュー方式: 複数コマンドを順番に実行
- B) コンポジットコマンド: 複数コマンドを1つにまとめる
- C) 状態マシン追加: コマンドの実行条件を状態で管理

**あなたの考え**:
```
[ここに記入してください]




```

---

### 2. ターゲット管理の複雑化

**問題**: 複数のターゲット（最も近い敵、最もHPが低い敵など）をどう扱うか

**現状の設計**:
```cpp
Entity* target_;  // 単一ターゲットのみ
```

**考えられる解決策**:
- A) ターゲット選択コマンド: `SelectTargetCommand`を追加
- B) ターゲットマネージャー: 外部でターゲット管理
- C) コマンド内で動的検索: Execute時に最適ターゲットを探索

**あなたの考え**:
```
[ここに記入してください]




```

---

### 3. コマンドのメモリ管理

**問題**: 大量の敵が同じコマンドを持つ場合、メモリが無駄になる

**現状の設計**:
```cpp
// 各Entityが独自のコマンドインスタンスを保持
std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
```

**考えられる解決策**:
- A) Flyweightパターン: 共有可能なコマンドを共有
- B) コマンドプール: コマンドを使い回す
- C) 現状維持: メモリ量が許容範囲なら変更不要

**あなたの考え**:
```
[ここに記入してください]




```

---

### 4. コマンドの中断とキャンセル

**問題**: 実行中のコマンド（ブリンク移動中など）を中断する方法がない

**現状の設計**:
```cpp
// コマンドは完了するまで実行される
void Execute(Entity* executor, Entity* target, float deltaTime);
```

**考えられる解決策**:
- A) Cancelメソッド追加: `virtual void Cancel()`
- B) 優先度システム: 高優先度コマンドが低優先度を中断
- C) 状態フラグ: `isInterruptible`フラグで制御

**あなたの考え**:
```
[ここに記入してください]




```

---

### 5. アニメーションとの同期

**問題**: コマンド実行とアニメーション再生のタイミング制御

**現状の設計**:
```cpp
// コマンド内でアニメーション制御が必要
void Execute(...) {
    // TODO: アニメーション再生
}
```

**考えられる解決策**:
- A) コールバック方式: アニメーション終了時にコールバック
- B) AnimationCommand: アニメーション専用コマンド
- C) イベントシステム: コマンドがイベントを発火

**あなたの考え**:
```
[ここに記入してください]




```

---

### 6. デバッグとプロファイリング

**問題**: どのコマンドがパフォーマンスボトルネックか分析しにくい

**現状の設計**:
```cpp
// 実行時間の計測機能がない
void ExecuteCommand(const std::string& name, float deltaTime);
```

**考えられる解決策**:
- A) プロファイリング機能: 各コマンドの実行時間を記録
- B) デバッグモード: 詳細ログを出力
- C) 統計情報: コマンドごとの呼び出し回数と平均時間

**あなたの考え**:
```
[ここに記入してください]




```

---

### 7. JSON設定の検証

**問題**: 不正なJSON設定（存在しないコマンドタイプなど）のエラー処理

**現状の設計**:
```cpp
// エラー時にnullptrを返すが、ランタイムエラーの可能性
if (type == "LinearMove") { ... }
else if (type == "BlinkMove") { ... }
return nullptr;  // 未知のタイプ
```

**考えられる解決策**:
- A) スキーマ検証: JSONスキーマで事前検証
- B) デフォルトコマンド: 不正時はデフォルト動作
- C) 起動時チェック: 全JSON設定を起動時に検証

**あなたの考え**:
```
[ここに記入してください]




```

---

## 次のステップ

上記の懸念点を検討後、以下の拡張が可能です：

1. **追加コマンド実装**
   - PatrolMoveCommand（巡回移動）
   - FlankMoveCommand（側面攻撃移動）
   - RangedAttackCommand（遠距離攻撃）
   - DashCommand（突進）

2. **高度な機能**
   - コマンドキューシステム
   - 優先度ベースのコマンド選択
   - アニメーションイベント連携
   - ネットワーク同期対応

3. **ツール開発**
   - JSON設定のGUIエディタ
   - リアルタイムパラメータ調整
   - コマンド動作のビジュアライザ

---

## 参考: 新しいコマンドの追加手順

1. **コマンドクラス作成**
   ```cpp
   // Game/Commands/Move/YourCommand.hpp
   class YourCommand : public ICommand {
       struct Config { /* パラメータ */ };
       void Execute(...) override { /* 実装 */ }
   };
   ```

2. **CommandFactoryに登録**
   ```cpp
   if (type == "YourCommand") {
       return std::make_unique<YourCommand>(config);
   }
   ```

3. **JSON設定作成**
   ```json
   {
     "commands": {
       "move": {
         "type": "YourCommand",
         "param1": 123
       }
     }
   }
   ```

4. **テスト**
   - プレイヤーに適用して動作確認
   - 問題なければ敵に適用

---

## まとめ

この設計により：
- ✅ Enemyクラスを一切変更せずに新しい動作を追加可能
- ✅ JSON設定のみで敵のバリエーションを無限に作成可能
- ✅ プレイヤーでテストして開発速度向上
- ✅ デバッグ情報が明確で問題の特定が容易

**開発コストが低く、拡張性が高い**システムです。
