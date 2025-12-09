# ゲーム開発における移動システムの設計パターン分析

## 📋 目次

1. [現在の実装状況](#現在の実装状況)
2. [ゲーム業界の一般的なパターン](#ゲーム業界の一般的なパターン)
3. [パフォーマンス vs 実装の速さ vs 扱いやすさ](#パフォーマンス-vs-実装の速さ-vs-扱いやすさ)
4. [メモリ効率化の具体的手法](#メモリ効率化の具体的手法)
5. [推奨設計: 最適化されたハイブリッドアプローチ](#推奨設計-最適化されたハイブリッドアプローチ)
6. [実装ロードマップ](#実装ロードマップ)

---

## 現在の実装状況

### アーキテクチャ図

```
┌─────────────────────────────────────────────────────────┐
│                       Player                            │
├─────────────────────────────────────────────────────────┤
│ - PlayerInputProvider (Input → MoveInputData)          │
│ - Movement (IMovementBehavior実行)                     │
│   └─ WalkBehavior, DashBehavior, etc.                  │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                       Enemy                             │
├─────────────────────────────────────────────────────────┤
│ - EnemyInputProvider (AI → MoveInputData)              │
│ - Movement (IMovementBehavior実行)                     │
│ - ICommand (AI思考ロジック)                            │
│   └─ ToTargetCommand → SetMoveInput()                  │
└─────────────────────────────────────────────────────────┘
```

### 実行フロー

```cpp
// Enemy::UpdateMovement()
1. inputProvider_->Clear()
2. moveCommand_->Execute()  // AI思考 → SetMoveInput()
3. movement_->Update()      // Behaviorで移動実行
```

### 現在の問題点

| 問題 | 詳細 | 影響 |
|------|------|------|
| **メモリ重複** | 各Enemyが独自の`Movement`, `InputProvider`, `Command`を所有 | 100体で300オブジェクト |
| **役割の重複** | `Command`が思考、`Movement`が実行だが境界が曖昧 | 保守性低下 |
| **Behaviorの非共有** | 各Enemyが独自の`WalkBehavior`インスタンス | メモリ非効率 |

---

## ゲーム業界の一般的なパターン

### 1. ECS (Entity Component System)

**代表例**: Unity DOTS, Unreal Mass, Bevy (Rust)

```
Entity: ID のみ
Component: データのみ (位置、速度、入力など)
System: ロジックのみ (MovementSystem, AISystem)

┌─────────────┐
│ MovementSystem (ロジック)
├─────────────┤
│ for entity in entities:
│   velocity = GetComponent<Velocity>(entity)
│   input = GetComponent<Input>(entity)
│   velocity.value = CalculateMovement(input)
└─────────────┘
```

**メリット**:
- ✅ **最高のパフォーマンス**: キャッシュ効率、並列処理
- ✅ **最高のメモリ効率**: データ指向設計

**デメリット**:
- ❌ **学習コスト高**: パラダイムシフトが必要
- ❌ **実装時間大**: 既存コードの大規模リファクタリング
- ❌ **デバッグ困難**: オブジェクト指向的な追跡が難しい

**適用判断**:
- ✅ 数千〜数万のエンティティ
- ❌ 数十〜数百のエンティティ（オーバーエンジニアリング）

---

### 2. Component Pattern (Unity風)

**代表例**: Unity MonoBehaviour, Unreal Actor Components

```cpp
class GameObject {
    std::vector<IComponent*> components_;

    template<typename T>
    T* GetComponent();

    void Update(float dt) {
        for (auto* comp : components_) {
            comp->Update(dt);
        }
    }
};

class MovementComponent : public IComponent {
    void Update(float dt) override { /* 移動処理 */ }
};

class AIComponent : public IComponent {
    void Update(float dt) override { /* AI思考 */ }
};
```

**メリット**:
- ✅ **柔軟性**: コンポーネント組み合わせで多様性
- ✅ **実装の速さ**: 直感的で理解しやすい
- ✅ **デバッグ容易**: オブジェクト単位で追跡

**デメリット**:
- ⚠️ **メモリ効率中**: 各GameObjectがコンポーネント所有
- ⚠️ **パフォーマンス中**: 仮想関数呼び出しのオーバーヘッド

**適用判断**:
- ✅ 中規模プロジェクト（数百エンティティ）
- ✅ プロトタイピング重視

---

### 3. Flyweight Pattern (共有オブジェクト)

**代表例**: テクスチャ、モデル、サウンドの共有

```cpp
class EnemyManager {
    // ⭐ 共有リソース
    std::unique_ptr<AIBehavior> chaseAI_;
    std::unique_ptr<AIBehavior> patrolAI_;
    std::unique_ptr<Movement> sharedMovement_;

    std::vector<Enemy*> enemies_;
};

class Enemy {
    // ⭐ 参照のみ（所有しない）
    AIBehavior* ai_ = nullptr;
    Movement* movement_ = nullptr;

    // ⭐ 個別データのみ所有
    Vector3 position_;
    float health_;
};
```

**メリット**:
- ✅ **最高のメモリ効率**: 共有可能なものは1つだけ
- ✅ **実装の速さ**: 既存コードへの適用が容易

**デメリット**:
- ⚠️ **状態管理注意**: 共有オブジェクトがステートフルだと破綻

**適用判断**:
- ✅ 同じ動作をする大量のエンティティ
- ✅ メモリ制約がある環境

---

### 4. State Machine (状態遷移)

**代表例**: ゲームAI、アニメーション制御

```cpp
class EnemyAI {
    enum State { IDLE, CHASE, ATTACK, FLEE };
    State currentState_ = IDLE;

    void Update(float dt) {
        switch (currentState_) {
            case IDLE: UpdateIdle(); break;
            case CHASE: UpdateChase(); break;
            case ATTACK: UpdateAttack(); break;
            case FLEE: UpdateFlee(); break;
        }
    }
};
```

**メリット**:
- ✅ **シンプル**: 理解しやすい
- ✅ **高速**: switch文の最適化

**デメリット**:
- ⚠️ **拡張性低**: 状態が増えると複雑化
- ⚠️ **遷移条件が散在**: 保守が困難

**適用判断**:
- ✅ 5〜10状態程度のシンプルなAI
- ❌ 複雑な状態遷移（Behavior Tree推奨）

---

### 5. Behavior Tree (階層的AI)

**代表例**: Unreal AI, HALO, The Sims

```
        Selector
       /    |    \
   Chase  Patrol  Idle
```

**メリット**:
- ✅ **複雑なAI**: 階層的で管理しやすい
- ✅ **再利用性**: ノード単位で共有

**デメリット**:
- ❌ **実装コスト高**: フレームワーク構築が必要
- ❌ **オーバーヘッド**: ツリー走査のコスト

**適用判断**:
- ✅ 複雑なAI（RPG、ステルスゲーム）
- ❌ シンプルな追跡AI（オーバーエンジニアリング）

---

## パフォーマンス vs 実装の速さ vs 扱いやすさ

### 比較表

| パターン | パフォーマンス | 実装の速さ | 扱いやすさ | メモリ効率 | 総合評価 |
|---------|--------------|-----------|-----------|----------|---------|
| **ECS** | ⭐⭐⭐⭐⭐ | ⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ | 大規模向き |
| **Component** | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | 中規模向き |
| **Flyweight** | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | **推奨** |
| **State Machine** | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ | シンプルAI向き |
| **Behavior Tree** | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ | 複雑AI向き |

### 現在のプロジェクトへの適合性

```
規模: 敵100体程度（中規模）
目標: パフォーマンス + 実装速度 + 扱いやすさのバランス
制約: 既存のPlayer/Enemyシステムを大きく変更したくない

→ 推奨: Flyweight Pattern + 現在のInputProvider設計
```

---

## メモリ効率化の具体的手法

### 現在のメモリ使用量（100体の場合）

```
Enemy 1体あたり:
- Movement: ~64 bytes (vector<unique_ptr>, vtable)
- EnemyInputProvider: ~32 bytes (MoveInputData)
- ICommand: ~16 bytes (vtable)
- WalkBehavior: ~48 bytes (speed, state, vtable)

合計: ~160 bytes × 100体 = 16,000 bytes (16KB)
```

### 最適化後のメモリ使用量

```
共有リソース（EnemyManager内）:
- Movement: 64 bytes × 1 = 64 bytes
- WalkBehavior: 48 bytes × 1 = 48 bytes
- DashBehavior: 64 bytes × 1 = 64 bytes
- 各種Command: ~16 bytes × 4 = 64 bytes

Enemy 1体あたり:
- EnemyInputProvider: 32 bytes
- ポインタ: 8 bytes × 2 = 16 bytes

合計: 240 bytes (共有) + 48 bytes × 100体 = 5,040 bytes (5KB)

削減率: 68% (16KB → 5KB)
```

---

## 推奨設計: 最適化されたハイブリッドアプローチ

### コンセプト

1. **Movementを共有** (Flyweight)
2. **Behaviorを共有** (Flyweight)
3. **InputProviderは個別** (各Enemyの状態)
4. **Commandは共有** (Flyweight)

### アーキテクチャ

```cpp
// ========================================
// 1. EnemyManager（共有リソース管理）
// ========================================
class EnemyManager {
    // ⭐ 共有Movement（1つだけ）
    std::unique_ptr<Movement> sharedMovement_;

    // ⭐ 共有Behavior（各種1つずつ）
    std::unique_ptr<IMovementBehavior> walkBehavior_;
    std::unique_ptr<IMovementBehavior> dashBehavior_;

    // ⭐ 共有Command（AI思考ロジック）
    std::unique_ptr<ICommand> toTargetCommand_;
    std::unique_ptr<ICommand> spiralCommand_;
    std::unique_ptr<ICommand> dashCommand_;

    std::vector<std::unique_ptr<Enemy>> enemies_;

public:
    void Initialize() {
        // Movementを1つだけ生成
        sharedMovement_ = std::make_unique<Movement>();

        // Behaviorを各種1つずつ生成
        walkBehavior_ = std::make_unique<WalkBehavior>(3.0f);
        dashBehavior_ = std::make_unique<DashBehavior>(10.0f, 0.3f, 1.0f);

        // Commandを各種1つずつ生成
        toTargetCommand_ = std::make_unique<ToTargetCommand>(3.0f, 0.5f);
        spiralCommand_ = std::make_unique<SpiralCommand>(2.5f, 3.0f);
        dashCommand_ = std::make_unique<DashCommand>();
    }

    void Spawn() {
        auto enemy = std::make_unique<Enemy>();
        enemy->Initialize();

        // ⭐ 共有リソースへの参照を設定
        enemy->SetMovement(sharedMovement_.get());
        enemy->SetMoveCommand(toTargetCommand_.get());  // ランダムに選択

        enemies_.push_back(std::move(enemy));
    }

    void Update(float dt) {
        // ⭐ Behaviorを動的に切り替え可能
        for (auto& enemy : enemies_) {
            // 距離に応じてBehaviorを切り替え
            float distance = GetDistanceToPlayer(enemy.get());
            if (distance < 3.0f) {
                sharedMovement_->ClearBehaviors();
                sharedMovement_->AddBehavior(dashBehavior_.get());  // ポインタのみ
            } else {
                sharedMovement_->ClearBehaviors();
                sharedMovement_->AddBehavior(walkBehavior_.get());
            }

            enemy->Update(dt);
        }
    }
};

// ========================================
// 2. Enemy（個別状態のみ保持）
// ========================================
class Enemy : public GameObject {
    // ⭐ 個別データ（各Enemyが所有）
    std::unique_ptr<EnemyInputProvider> inputProvider_;

    // ⭐ 参照データ（共有、所有しない）
    Movement* movement_ = nullptr;
    ICommand* moveCommand_ = nullptr;
    GameObject* target_ = nullptr;

public:
    void Initialize() {
        // 個別のInputProviderのみ生成
        inputProvider_ = std::make_unique<EnemyInputProvider>();
    }

    void SetMovement(Movement* movement) {
        movement_ = movement;
        // ⭐ 共有Movementに自分のInputProviderを設定
        if (movement_) {
            movement_->SetInputProvider(inputProvider_.get());
        }
    }

    void SetMoveCommand(ICommand* command) {
        moveCommand_ = command;
    }

    void UpdateMovement(float deltaTime) {
        if (knockback_) {
            UpdateKnockback(deltaTime);
        } else {
            // AI思考
            if (inputProvider_) inputProvider_->Clear();
            if (moveCommand_ && target_) {
                moveCommand_->Execute(this, target_, deltaTime);
            }

            // Movement実行
            if (movement_) {
                movement_->Update(deltaTime);
            }

            if (invincible_) {
                velocity_ *= 0.3f;
            }
        }
    }
};

// ========================================
// 3. Movement（共有可能に改良）
// ========================================
class Movement {
    std::vector<IMovementBehavior*> behaviors_;  // ⭐ ポインタに変更（共有）
    IInputProvider* inputProvider_ = nullptr;
    GameObject* owner_ = nullptr;

public:
    // ⭐ Behaviorをポインタで追加（所有しない）
    void AddBehavior(IMovementBehavior* behavior) {
        if (behavior) {
            behaviors_.push_back(behavior);
        }
    }

    void ClearBehaviors() {
        behaviors_.clear();
    }

    void Update(float deltaTime) {
        if (!owner_ || !inputProvider_) return;

        const auto& inputData = inputProvider_->GetInput();

        // 優先度順にソート
        std::ranges::sort(behaviors_,
            [](IMovementBehavior* a, IMovementBehavior* b) {
                return a->GetPriority() > b->GetPriority();
            });

        // 実行可能な動作を適用
        for (auto* behavior : behaviors_) {
            if (behavior->CanExecute(inputData)) {
                Vector3 velocity = behavior->Calculate(inputData, deltaTime);
                owner_->SetVelocity(velocity);
                return;
            }
        }

        owner_->SetVelocity({});
    }
};
```

### 🚨 注意点: Movementの共有問題

**問題**: 1つのMovementを全Enemyで共有すると、`owner_`と`inputProvider_`が上書きされる

**解決策A: Movementは共有しない**

```cpp
// EnemyManager
std::unique_ptr<IMovementBehavior> walkBehavior_;  // 共有
std::unique_ptr<IMovementBehavior> dashBehavior_;  // 共有

// Enemy
std::unique_ptr<Movement> movement_;  // 個別（軽量）
std::unique_ptr<EnemyInputProvider> inputProvider_;  // 個別

void Enemy::Initialize() {
    inputProvider_ = std::make_unique<EnemyInputProvider>();
    movement_ = std::make_unique<Movement>();
    movement_->Initialize(this);
    movement_->SetInputProvider(inputProvider_.get());

    // ⭐ 共有Behaviorを参照として追加
    movement_->AddBehavior(manager->GetWalkBehavior());
}
```

**メモリ影響**:
```
Movement 1つ: ~64 bytes
100体で: 6.4KB

共有Behavior: ~200 bytes (全種類合計)

合計: 6.6KB（十分に軽量）
```

**解決策B: Movementを完全に共有（上級者向け）**

```cpp
class Movement {
    std::vector<IMovementBehavior*> behaviors_;

    // ⭐ owner/inputProviderを引数で受け取る
    void Update(GameObject* owner, IInputProvider* input, float deltaTime) {
        if (!owner || !input) return;

        const auto& inputData = input->GetInput();

        for (auto* behavior : behaviors_) {
            if (behavior->CanExecute(inputData)) {
                Vector3 velocity = behavior->Calculate(inputData, deltaTime);
                owner->SetVelocity(velocity);
                return;
            }
        }
    }
};

// Enemy::UpdateMovement()
if (movement_) {
    movement_->Update(this, inputProvider_.get(), deltaTime);
}
```

**メリット**: Movement完全共有で~64 bytes節約
**デメリット**: APIが複雑化

---

### 推奨: 解決策A（Behavior共有 + Movement個別）

**理由**:
1. ✅ **実装が簡単**: 既存コードの変更が最小限
2. ✅ **メモリ効率良い**: Behaviorが重い（100体で~4.8KB削減）
3. ✅ **バグが少ない**: 各Enemyが独立したMovementを持つ
4. ✅ **パフォーマンス良い**: 共有Behaviorはキャッシュ効率高

---

## 実装ロードマップ

### Phase 1: Behavior共有化（推奨）

**所要時間**: 2-3時間

**手順**:
1. `Movement::AddBehavior()`を`unique_ptr`から生ポインタに変更
2. `EnemyManager`に共有Behaviorを追加
3. `Enemy::Initialize()`で共有Behaviorを参照

**効果**:
- メモリ削減: ~4.8KB (100体)
- 実装コスト: 低
- リスク: 低

### Phase 2: Command共有化

**所要時間**: 1-2時間

**手順**:
1. `EnemyManager`に共有Commandを追加
2. `Enemy::SetMoveCommand()`を生ポインタに変更
3. Spawn時にランダムにCommandを割り当て

**効果**:
- メモリ削減: ~1.5KB (100体)
- 移動バリエーション追加が容易

### Phase 3: 動的Behavior切り替え（オプション）

**所要時間**: 3-4時間

**手順**:
1. `EnemyManager::Update()`で状況に応じてBehavior切り替え
2. 距離、HP、フェーズなどで判定

**効果**:
- AIの多様性向上
- 就活アピール強化

---

## まとめ

### ゲーム開発で最もよく使われる手法

1. **中小規模（〜1000体）**: Flyweight Pattern + Component Pattern
2. **大規模（1000体〜）**: ECS (Unity DOTS, Unreal Mass)
3. **AI重視**: Behavior Tree + State Machine

### あなたのプロジェクトへの推奨

**最適解**: **Flyweight Pattern（Behavior共有）**

**理由**:
- ✅ パフォーマンス: 高（メモリ効率、キャッシュ効率）
- ✅ 実装の速さ: 高（既存コードへの変更少）
- ✅ 扱いやすさ: 高（直感的、デバッグ容易）
- ✅ DRY原則: Player/Enemyで同じBehavior使用

### 実装優先度

```
優先度1: Behavior共有化 (Phase 1)
  → メモリ削減 + DRY達成

優先度2: Command共有化 (Phase 2)
  → 移動バリエーション追加

優先度3: 動的切り替え (Phase 3)
  → AIの複雑化（就活アピール）
```

### メモリ効率の現実

```
最適化前: 16KB (100体)
最適化後: 6.6KB (100体)
削減率: 58.75%

削減量: 9.4KB ≈ 小さな画像1枚分

結論: 現代のゲームでは誤差レベルだが、
      設計の良さとして就活でアピール可能
```

### 最終提案

**すぐに実装**: Phase 1（Behavior共有）
**余裕があれば**: Phase 2（Command共有）
**将来の拡張**: Phase 3（動的切り替え）

この順序で段階的に実装することで、リスクを最小化しながらメモリ効率とコード品質を向上できます。
