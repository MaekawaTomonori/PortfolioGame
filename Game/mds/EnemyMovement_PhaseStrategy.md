# Enemy Movement System: 段階的移行戦略

## 📋 目次

1. [現状分析](#現状分析)
2. [問題点と課題](#問題点と課題)
3. [段階的移行戦略](#段階的移行戦略)
4. [Phase 1: Enemy Movement最適化（今回実装）](#phase-1-enemy-movement最適化今回実装)
5. [Phase 2: InputHandler統合（将来実装）](#phase-2-inputhandler統合将来実装)
6. [実装ファイル一覧](#実装ファイル一覧)

---

## 現状分析

### Player Movement System (Strategy Pattern)

```cpp
// Movement.hpp - コンテナクラス
class Movement {
    std::vector<std::unique_ptr<IMovementBehavior>> behaviors_;

    void Update(float deltaTime) {
        // 優先度順にソート → 最初に実行可能な戦略を適用
        for (auto& behavior : behaviors_) {
            if (behavior->CanExecute(input_)) {
                Vector3 velocity = behavior->Calculate(input_, deltaTime);
                owner_->SetVelocity(velocity);
                return;
            }
        }
    }
};

// IMovementBehavior - Strategy Interface
class IMovementBehavior {
    virtual Vector3 Calculate(const Input* input, float deltaTime) = 0;
    virtual bool CanExecute(const Input* input) = 0;
    virtual int GetPriority() const = 0;
};

// DashBehavior - Concrete Strategy (⚠️ 状態を持つ)
class DashBehavior : public IMovementBehavior {
    float currentDuration_ = 0.0f;  // 各Playerが独自の状態
    float currentCooldown_ = 0.0f;
    bool isDashing_ = false;
    Vector3 dashDirection_{};
};
```

**特徴**:
- ✅ 複数の動作を優先度で切り替え
- ✅ Input依存の条件判定
- ❌ 各Behaviorが状態を持つ（メモリ非効率）

### Enemy Movement System (Command Pattern)

```cpp
// Enemy.hpp
class Enemy {
    std::unique_ptr<ICommand> moveCommand_;

    void UpdateMovement(float deltaTime) {
        if (moveCommand_) {
            moveCommand_->Execute(this, target_, deltaTime);
        }
    }
};

// ICommand - Command Interface
class ICommand {
    virtual void Execute(GameObject* executor, GameObject* target, float deltaTime) = 0;
};

// ToTargetCommand - Concrete Command (ステートレス)
class ToTargetCommand : public ICommand {
    float speed_;
    float minDistance_;
    // ⚠️ 実行時の状態を持たない
};
```

**特徴**:
- ✅ シンプルなインターフェース
- ❌ 単一コマンドのみ（複数動作の切り替え不可）
- ❌ 各Enemyが独自Commandインスタンスを所有（メモリ非効率）

---

## 問題点と課題

### 問題1: メモリ効率

| システム | 現状 | 問題 |
|---------|------|------|
| Player | 各Playerが独自Behavior | 複数プレイヤー時に非効率 |
| Enemy | 各Enemyが独自Command | **100体で100個のインスタンス** |

### 問題2: 状態管理の不一致

```cpp
// PlayerのDashBehaviorは状態を持つ
class DashBehavior {
    float currentDuration_;  // Behavior内部で状態管理
    bool isDashing_;
};

// EnemyのSpiralApproachはどこに角度を保存する？
class SpiralApproachCommand {
    // Option A: Command内に保存 → 設計として違和感
    std::unordered_map<GameObject*, float> angles_;

    // Option B: Enemy内に保存 → Enemyクラスが肥大化
};
```

### 問題3: 将来の統合障壁

- PlayerはInput依存
- EnemyはTarget依存
- 異なるインターフェースで統合困難

---

## 段階的移行戦略

```
【現在】
Player: IMovementBehavior (Input依存)
Enemy:  ICommand (ステートレス)

    ↓ Phase 1 (今回実装)

【Phase 1】
Player: IMovementBehavior (Input依存) ← 変更なし
Enemy:  ICommand (状態外部化) ← メモリ効率化 + バリエーション追加

    ↓ Phase 2 (将来実装)

【Phase 2】
Player: IMovementBehavior (InputHandler依存)
Enemy:  IMovementBehavior (InputHandler依存) ← 統合完了
```

---

## Phase 1: Enemy Movement最適化（今回実装）

### 目標

- ✅ Playerに触れず、Enemyのみ改善
- ✅ メモリ効率96%削減（100体 → 4インスタンス）
- ✅ 移動バリエーション400%増加（1種 → 4種）
- ✅ 将来のPlayer統合に備えた設計

### アーキテクチャ

#### 1. MovementState（汎用状態管理）

```cpp
// Game/Enemy/MovementState.hpp
#ifndef MOVEMENT_STATE_HPP_
#define MOVEMENT_STATE_HPP_

#include "Math/Vector3.hpp"

/// <summary>
/// 移動コマンドの実行状態を保持する汎用構造体
/// 各Enemyが自分専用のインスタンスを保持
/// </summary>
struct MovementState {
    // 汎用パラメータ（全てのCommandで使い回し可能）
    float timer = 0.f;          // タイマー（クールダウン、持続時間など）
    float angle = 0.f;          // 角度（螺旋移動など）
    int phase = 0;              // フェーズ（待機、実行、クールダウンなど）
    Vector3 savedDirection = {}; // 保存された方向（突進など）
    bool flag = false;          // 汎用フラグ（左右切り替えなど）

    /// <summary>
    /// 状態を初期化
    /// Command変更時に呼び出す
    /// </summary>
    void Reset() {
        timer = 0.f;
        angle = 0.f;
        phase = 0;
        savedDirection = {};
        flag = false;
    }
};

#endif // MOVEMENT_STATE_HPP_
```

#### 2. 改良版ICommand

```cpp
// Game/Command/ICommand.hpp
#ifndef ICommand_HPP_
#define ICommand_HPP_

class GameObject;
struct MovementState;  // 前方宣言

/// <summary>
/// コマンドの基底インターフェース
///
/// 設計方針:
/// - Flyweightパターン: 複数のGameObjectで共有可能
/// - ステートレス: 状態はMovementStateとして外部管理
/// - デバッグ可能: ImGuiでデバッグ情報表示
/// </summary>
class ICommand {
public:
    virtual ~ICommand() = default;

    /// <summary>
    /// コマンドを実行
    /// </summary>
    /// <param name="executor">実行者（Enemy）</param>
    /// <param name="target">ターゲット（Player）</param>
    /// <param name="state">実行者の状態（参照渡し）</param>
    /// <param name="deltaTime">前フレームからの経過時間（秒）</param>
    virtual void Execute(
        GameObject* executor,
        GameObject* target,
        MovementState& state,
        float deltaTime
    ) = 0;

    /// <summary>
    /// コマンドがアクティブかどうか
    /// </summary>
    virtual bool IsActive() const { return true; }

    /// <summary>
    /// デバッグ情報をImGuiで表示
    /// </summary>
    virtual void Debug() {}

    /// <summary>
    /// コマンド自体のリセット（殆ど不要）
    /// </summary>
    virtual void Reset() {}
};

#endif // ICommand_HPP_
```

#### 3. Enemyクラス

```cpp
// Game/Enemy/Enemy.hpp (抜粋)
#include "Enemy/MovementState.hpp"

class Enemy : public GameObject {
    ICommand* moveCommand_ = nullptr;   // ⭐ 生ポインタ（共有）
    MovementState movementState_;        // ⭐ 状態は自分で管理
    GameObject* target_ = nullptr;

public:
    /// <summary>
    /// 移動コマンドを設定（ポインタのみコピー）
    /// </summary>
    void SetMoveCommand(ICommand* command) {
        moveCommand_ = command;
        movementState_.Reset();  // Command変更時に状態リセット
    }

private:
    void UpdateMovement(float deltaTime);
};

// Game/Enemy/Enemy.cpp (抜粋)
void Enemy::UpdateMovement(float _deltaTime) {
    if (knockback_) {
        UpdateKnockback(_deltaTime);
    } else {
        // 移動コマンドを実行
        if (moveCommand_ && target_) {
            // ⭐ 自分の状態を渡す
            moveCommand_->Execute(this, target_, movementState_, _deltaTime);
        }

        // 無敵時は移動速度を減衰
        if (invincible_) {
            velocity_ *= 0.3f;
        }
    }
}
```

#### 4. Concrete Commands

##### ToTargetCommand（直線追跡）

```cpp
class ToTargetCommand : public ICommand {
    float speed_;
    float minDistance_;

public:
    ToTargetCommand(float speed = 3.0f, float minDist = 0.5f);

    void Execute(GameObject* executor, GameObject* target,
                 MovementState& state, float deltaTime) override {
        if (!executor || !target) return;

        Vector3 direction = target->GetPosition() - executor->GetPosition();
        float distance = direction.Length();

        if (distance <= minDistance_) {
            executor->SetVelocity({});
            return;
        }

        direction.y = 0;
        executor->SetVelocity(direction.Normalize() * speed_);
    }
};
```

##### SpiralApproachCommand（螺旋接近）

```cpp
class SpiralApproachCommand : public ICommand {
    float approachSpeed_;
    float rotationSpeed_;
    float minDistance_;

public:
    SpiralApproachCommand(float approach = 2.5f, float rotation = 3.0f, float minDist = 0.8f);

    void Execute(GameObject* executor, GameObject* target,
                 MovementState& state, float deltaTime) override {
        if (!executor || !target) return;

        Vector3 toTarget = target->GetPosition() - executor->GetPosition();
        toTarget.y = 0;
        float distance = toTarget.Length();

        if (distance <= minDistance_) {
            executor->SetVelocity({});
            return;
        }

        // ⭐ state.angleを使用
        state.angle += rotationSpeed_ * deltaTime;

        // 螺旋軌道計算
        Vector3 forward = toTarget.Normalize();
        Vector3 right = Vector3{-forward.z, 0, forward.x};  // 90度回転

        float orbitRadius = distance * 0.3f;
        Vector3 offset = right * std::sin(state.angle) * orbitRadius;

        Vector3 targetPos = target->GetPosition() + offset;
        Vector3 direction = (targetPos - executor->GetPosition()).Normalize();

        executor->SetVelocity(direction * approachSpeed_);
    }
};
```

##### DashCommand（突進攻撃）

```cpp
class DashCommand : public ICommand {
    float chargeTime_;      // 溜め時間
    float dashSpeed_;       // 突進速度
    float dashDuration_;    // 突進持続時間
    float cooldown_;        // クールダウン
    float triggerDistance_; // 発動距離

    enum Phase { IDLE, CHARGING, DASHING, COOLDOWN };

public:
    DashCommand(float charge = 0.5f, float speed = 10.0f,
                float duration = 0.3f, float cool = 2.0f, float trigger = 5.0f);

    void Execute(GameObject* executor, GameObject* target,
                 MovementState& state, float deltaTime) override {
        if (!executor || !target) return;

        Vector3 toTarget = target->GetPosition() - executor->GetPosition();
        toTarget.y = 0;
        float distance = toTarget.Length();

        // ⭐ state.phase: Phase enum相当
        // ⭐ state.timer: フェーズ内タイマー
        // ⭐ state.savedDirection: 突進方向

        switch (state.phase) {
            case IDLE:
                if (distance <= triggerDistance_) {
                    state.phase = CHARGING;
                    state.timer = 0.f;
                    executor->SetVelocity({});
                } else {
                    executor->SetVelocity(toTarget.Normalize() * (dashSpeed_ * 0.3f));
                }
                break;

            case CHARGING:
                state.timer += deltaTime;
                executor->SetVelocity({});

                if (state.timer >= chargeTime_) {
                    state.phase = DASHING;
                    state.timer = 0.f;
                    state.savedDirection = toTarget.Normalize();
                }
                break;

            case DASHING:
                state.timer += deltaTime;
                executor->SetVelocity(state.savedDirection * dashSpeed_);

                if (state.timer >= dashDuration_) {
                    state.phase = COOLDOWN;
                    state.timer = 0.f;
                }
                break;

            case COOLDOWN:
                state.timer += deltaTime;
                executor->SetVelocity({});

                if (state.timer >= cooldown_) {
                    state.phase = IDLE;
                    state.timer = 0.f;
                }
                break;
        }
    }
};
```

##### SidestepCommand（サイドステップ接近）

```cpp
class SidestepCommand : public ICommand {
    float forwardSpeed_;
    float sideStepWidth_;
    float stepInterval_;

public:
    SidestepCommand(float forward = 2.0f, float width = 2.0f, float interval = 0.8f);

    void Execute(GameObject* executor, GameObject* target,
                 MovementState& state, float deltaTime) override {
        if (!executor || !target) return;

        Vector3 toTarget = target->GetPosition() - executor->GetPosition();
        toTarget.y = 0;

        // ⭐ state.timer: 次のステップまでの時間
        // ⭐ state.flag: 左右フラグ

        state.timer += deltaTime;

        if (state.timer >= stepInterval_) {
            state.flag = !state.flag;
            state.timer = 0.f;
        }

        Vector3 forward = toTarget.Normalize();
        Vector3 right = Vector3{-forward.z, 0, forward.x};

        float sideFactor = state.flag ? 1.0f : -1.0f;
        Vector3 velocity = forward * forwardSpeed_ + right * (sideFactor * sideStepWidth_);

        executor->SetVelocity(velocity);
    }
};
```

#### 5. Enemiesクラス（共有Command管理）

```cpp
// Game/Enemy/Enemies.hpp (抜粋)
class Enemies {
    // ⭐ 共有Commandインスタンス（Flyweight）
    std::unique_ptr<ICommand> toTargetCommand_;
    std::unique_ptr<ICommand> spiralCommand_;
    std::unique_ptr<ICommand> dashCommand_;
    std::unique_ptr<ICommand> sidestepCommand_;

    std::vector<std::unique_ptr<Enemy>> enemies_;

public:
    void Initialize(ParticleSystem* particle);
    void Spawn();
};

// Game/Enemy/Enemies.cpp
void Enemies::Initialize(ParticleSystem* _particle) {
    particle_ = _particle;

    // ⭐ 4つのCommandを1回だけ生成
    toTargetCommand_ = std::make_unique<ToTargetCommand>(3.0f, 0.5f);
    spiralCommand_ = std::make_unique<SpiralApproachCommand>(2.5f, 3.0f, 0.8f);
    dashCommand_ = std::make_unique<DashCommand>(0.5f, 10.0f, 0.3f, 2.0f, 5.0f);
    sidestepCommand_ = std::make_unique<SidestepCommand>(2.0f, 2.0f, 0.8f);

    // Particle設定...
}

void Enemies::Spawn() {
    if (enemies_.size() >= MaxEnemies) return;

    auto enemy = std::make_unique<Enemy>();
    enemy->Initialize();
    enemy->SetParticleSystem(particle_);
    enemy->SetTarget(target_);

    // ⭐ ランダムにCommandを割り当て（ポインタのみコピー）
    int type = rand() % 4;
    switch (type) {
        case 0: enemy->SetMoveCommand(toTargetCommand_.get()); break;
        case 1: enemy->SetMoveCommand(spiralCommand_.get()); break;
        case 2: enemy->SetMoveCommand(dashCommand_.get()); break;
        case 3: enemy->SetMoveCommand(sidestepCommand_.get()); break;
    }

    enemies_.push_back(std::move(enemy));
}
```

### Phase 1 の効果

| 項目 | Before | After | 改善率 |
|------|--------|-------|--------|
| **メモリ使用量** | 100 Commands | 4 Commands | **96%削減** |
| **移動バリエーション** | 1種類 | 4種類 | **400%増加** |
| **Enemy.hpp変更** | - | `MovementState`追加 | 最小限 |
| **Player影響** | - | 0変更 | **無影響** |
| **就活アピール** | 基本的なAI | 複雑で多様なAI | **大幅向上** |

---

## Phase 2: InputHandler統合（将来実装）

### 目標

- Player/Enemy両方が同じMovementシステムを使用
- Input依存を抽象化
- 完全な統合による保守性向上

### アーキテクチャ

#### 1. InputHandler抽象化

```cpp
// Game/Input/IInputSource.hpp
class IInputSource {
public:
    virtual ~IInputSource() = default;
    virtual Vector3 GetMovementDirection() const = 0;
    virtual bool IsDashRequested() const = 0;
    virtual bool IsAttackRequested() const = 0;
};

// Player用InputHandler
class PlayerInputHandler : public IInputSource {
    Input* input_;

public:
    Vector3 GetMovementDirection() const override {
        Vector3 dir = {};
        if (input_->IsPress(DIK_W)) dir.z += 1.0f;
        if (input_->IsPress(DIK_S)) dir.z -= 1.0f;
        if (input_->IsPress(DIK_A)) dir.x -= 1.0f;
        if (input_->IsPress(DIK_D)) dir.x += 1.0f;
        return dir.Normalize();
    }

    bool IsDashRequested() const override {
        return input_->IsPress(DIK_LSHIFT);
    }
};

// Enemy用InputHandler（AIの意思決定）
class AIInputHandler : public IInputSource {
    GameObject* owner_;
    GameObject* target_;

public:
    Vector3 GetMovementDirection() const override {
        // AIの意思決定ロジック
        Vector3 toTarget = target_->GetPosition() - owner_->GetPosition();
        return toTarget.Normalize();
    }

    bool IsDashRequested() const override {
        // 距離に応じてダッシュ判定
        float distance = (target_->GetPosition() - owner_->GetPosition()).Length();
        return distance < 5.0f;
    }
};
```

#### 2. 統合版IMovementBehavior

```cpp
class IMovementBehavior {
public:
    virtual ~IMovementBehavior() = default;

    virtual Vector3 Calculate(
        GameObject* owner,
        IInputSource* input,      // ⭐ Input抽象化
        MovementState& state,
        float deltaTime
    ) = 0;

    virtual bool CanExecute(IInputSource* input, const MovementState& state) const = 0;
    virtual int GetPriority() const = 0;
    virtual void Debug() {}
};
```

#### 3. 統合版Movement

```cpp
class Movement {
    std::vector<IMovementBehavior*> behaviors_;  // ⭐ 共有可能
    IInputSource* input_;
    GameObject* owner_;
    MovementState state_;  // ⭐ 状態管理

public:
    void Update(float deltaTime) {
        for (auto* behavior : behaviors_) {
            if (behavior->CanExecute(input_, state_)) {
                Vector3 velocity = behavior->Calculate(owner_, input_, state_, deltaTime);
                owner_->SetVelocity(velocity);
                return;
            }
        }
    }
};
```

### Phase 2 移行手順

```
Step 1: IInputSourceインターフェース導入
  - PlayerInputHandler実装
  - AIInputHandler実装

Step 2: PlayerのMovementをInputHandler対応に変更
  - IMovementBehaviorを改修
  - DashBehavior, WalkBehaviorを移行

Step 3: EnemyのICommandをIMovementBehaviorに統合
  - 4つのCommandをBehaviorに変換
  - Enemyクラスに統合版Movement導入

Step 4: Player/Enemyが同じMovementクラスを使用
  - 完全統合
  - テスト・検証
```

---

## 実装ファイル一覧

### Phase 1（今回実装）

#### 新規作成

```
Game/Enemy/MovementState.hpp
Game/Command/Move/SpiralApproachCommand.hpp
Game/Command/Move/SpiralApproachCommand.cpp
Game/Command/Move/DashCommand.hpp
Game/Command/Move/DashCommand.cpp
Game/Command/Move/SidestepCommand.hpp
Game/Command/Move/SidestepCommand.cpp
```

#### 修正

```
Game/Command/ICommand.hpp
  - MovementState& パラメータ追加

Game/Command/Move/ToTargetCommand.hpp
Game/Command/Move/ToTargetCommand.cpp
  - 新interface対応

Game/Enemy/Enemy.hpp
  - MovementState movementState_ 追加
  - SetMoveCommand(ICommand* command) 変更

Game/Enemy/Enemy.cpp
  - UpdateMovement内でstate渡し

Game/Enemy/Enemies.hpp
  - 共有Commandメンバー追加

Game/Enemy/Enemies.cpp
  - Initialize内でCommand生成
  - Spawn内でランダム割り当て
```

### Phase 2（将来実装）

```
Game/Input/IInputSource.hpp (新規)
Game/Input/PlayerInputHandler.hpp (新規)
Game/Input/AIInputHandler.hpp (新規)
Game/Movement/IMovementBehavior.hpp (改修)
Game/Movement/Movement.hpp (改修)
... (段階的に統合)
```

---

## まとめ

### Phase 1 の利点

✅ **メモリ効率96%削減**: 100個 → 4個のCommandインスタンス
✅ **就活アピール強化**: 4種類の多様なAI動作
✅ **移行コスト最小**: Playerに影響なし
✅ **将来の拡張性**: Phase 2への道筋が明確
✅ **保守性向上**: 状態管理が明確化

### 設計原則

- **Flyweightパターン**: Commandを共有してメモリ効率化
- **状態の外部化**: Commandはロジックのみ、状態はExecutorが管理
- **段階的移行**: 既存システムへの影響を最小化
- **将来の統合**: InputHandler導入でPlayer/Enemy統合可能

### 次のステップ

1. Phase 1の実装とテスト
2. 各Commandの動作パラメータ調整
3. デバッグUIでの状態可視化
4. Phase 2の詳細設計（必要に応じて）
