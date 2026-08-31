# 自作UIシステム 設計引き継ぎメモ

このドキュメントは設計セッションの決定事項をまとめたもの。
ローカル環境で実エンジンを見ながら継続するための出発点として使う。

---

## 0. 確定した基本方針

| 項目 | 決定 |
|---|---|
| 拡張方式 | コンポジション（Element + Behavior 差し替え）。継承しない |
| レイアウト | アンカー方式と自動レイアウトの両対応（Measure / Arrange 2パス） |
| 描画 | 自前で頂点バッファ・バッチングまで管理。既存 Sprite / Text は消費側 |
| UI定義 | JSON でシリアライズ。将来的に自作エンジン側にエディタを用意 |
| クリッピング | Stencil で任意形状・回転対応。ただし軸平行矩形は高速パス |
| アニメーション | プロパティトラック方式（タイムライン） |
| 入力 | キーボード + マウス前提 |
| 解像度対応 | アンカーによる可変アスペクト完全対応 |
| ウィンドウ機能 | 重ね合わせ・移動・リサイズのみ（最小化・タブ・ドッキングは対象外） |
| HUD連携 | イベント通知（変化時のみ dirty を立て、更新は遅延） |

---

## 1. レイヤ構成

```
6. HUD / Window Manager   … 具体的な画面部品
5. Widget (Button, Slider, Window)
4. Animation (Clip / Track / Player)
3. Input (HitTest → Event → Focus)
2. Layout (Anchor / AutoLayout)
1. Element (階層 + 矩形 + 描画パーツ)
0. Renderer 抽象 (DrawList)
```

下位レイヤは上位を知らない。Window Manager も HUD も
「Element + Behavior の組み合わせ」で表現でき、専用の基底クラスを持たない。

---

## 2. Element

```cpp
class Element {
  // 階層
  Element* parent_;
  std::vector<ElementHandle> children_;

  // 矩形・変換（親ローカル）
  Rect rect_;
  Vec2 pivot_, anchorMin_, anchorMax_;
  Vec2 scale_;  float rotation_;

  // 状態（子へ伝播）
  bool  active_   = true;
  float opacity_  = 1.0f;
  int   zOrder_   = 0;

  // 見た目：0〜n個を順に描く
  std::vector<std::unique_ptr<IDrawable>> drawables_;

  // 振る舞い
  std::vector<std::unique_ptr<IBehavior>> behaviors_;

  // 入力
  HitTestMode hitTest_ = HitTestMode::Rect;

  // キャッシュ
  Mat3  worldMat_;  float worldOpacity_;  DirtyFlags dirty_;
};
```

**要点**

- Element は「透明な箱」。見た目は `IDrawable` の差し替えで表現する
- `ImageElement` / `TextElement` のような継承分割はしない
  （アイコン付きボタンのような合成で破綻するため）
- 子は `ElementHandle{ index, generation }` で保持する。
  生ポインタは 1 フレーム内のみ有効という規約
- 構造変更（追加・削除）はコマンドキューに積み、安全点でまとめて適用する

### IBehavior

```cpp
class IBehavior {
public:
  virtual const char* TypeName() const = 0;
  virtual const PropertyTable& Properties() const = 0;

  virtual void OnAttach(Element&) {}
  virtual void OnDetach(Element&) {}
  virtual void OnUpdate(Element&, float dt) {}
  virtual bool OnEvent(Element&, const UIEvent&) { return false; } // true = 消費
  virtual void OnLayoutChanged(Element&) {}
};
```

仮想関数を後から増やすと利用側が壊れるため、フックは最初に列挙しておく。

### ウィジェットは「組み立て手順」

```cpp
Element* MakeButton(Element& parent, ...) {
  auto* e = parent.CreateChild();
  e->AddDrawable<Sprite>(atlas.Get("btn_bg"));  // 9スライス
  e->AddDrawable<Text>("OK", font);
  e->AddBehavior<Clickable>();
  e->AddBehavior<VisualState>();
  return e;
}
```

Window は `Draggable + Resizable + Focusable + ClipContent` の合成。

---

## 3. プロパティ反射（最優先で作る土台）

エディタのインスペクタ、JSONシリアライズ、アニメのトラック指定は
すべて「プロパティを文字列/ID で指して読み書きする」機能。個別実装すると破綻する。

```cpp
enum class PropType { Bool, Int, Float, Vec2, Color, String, AssetRef };

struct PropertyDesc {
  const char* name;
  PropType    type;
  void      (*set)(void* obj, const Value&);
  Value     (*get)(const void* obj);
  float min, max;    // エディタのスライダ範囲
  bool  animatable;  // アニメのトラック候補に出すか
};
```

`REGISTER_BEHAVIOR(Clickable, PROP(repeat), PROP_RANGE(interval, 0, 5))`
のようなマクロで「名前 → ファクトリ + プロパティ表」をグローバル登録する。

この投資はデバッグ用ランタイムインスペクタでも回収される
（エディタとまったく同じコードが動く）。

---

## 4. レイアウト

```cpp
class ILayout {  // 親 Element が 0 or 1 個保持
  virtual Vec2 Measure(Element&, Vec2 avail) = 0;
  virtual void Arrange(Element&, Rect final) = 0;
};
```

- `ILayout == nullptr` → **アンカーモード**。
  子が `anchorMin/Max + offset` から自力で矩形を算出
- `StackLayout` / `FlexLayout` / `GridLayout` を挿すと → **自動モード**。
  親が `LayoutParams{ sizeMode, weight, margin }` を見て配置

同一ツリー内で混在可能。HUD は絶対配置、ウィンドウ内部は Flex、という併用ができる。

### アンカーの算出

```cpp
Vec2 aMin = parent.pos + parent.size * anchorMin;
Vec2 aMax = parent.pos + parent.size * anchorMax;
// anchorMin == anchorMax → サイズ固定、offset は中心位置（ピン留め）
// anchorMin != anchorMax → 追従ストレッチ、offset は各辺のマージン
```

- `referenceResolution` と `uiScale` を持ち、固定サイズ要素とフォントに乗算
- `matchWidthOrHeight` を 0〜1 で保持し、幅基準／高さ基準／中間を切替
- ピクセルスナップは最終ワールド座標を round。
  ただし**アニメーション中の要素では無効化**する（カクつくため）

---

## 5. 更新順序と Dirty 伝播

```
① 入力ディスパッチ
② コマンドキュー flush（構造変更をここで適用）
③ Behavior::OnUpdate / アニメーション適用
④ Layout（Measure → Arrange、dirty な部分木のみ）
⑤ ワールド行列・不透明度の伝播
⑥ DrawList 構築（ソート・バッチ）
⑦ Emit
```

### DirtyFlags は伝播方向が異なる（最重要）

| フラグ | 契機 | 伝播方向 |
|---|---|---|
| `Layout` | サイズ制約・子構成の変化 | **親へ（上方向）** |
| `Transform` | 位置・回転・スケール | **子へ（下方向）** |
| `Content` | 色・テキスト・画像の差し替え | 伝播しない |
| `Order` | Zオーダー変更 | 親のソートのみ |

この非対称性を混同すると相互再計算で無限ループする。

- 上方向の停止条件：親が固定サイズ（子に依存しない）ならそこで打ち切る
- Measure キャッシュ：同じ `availableSize` なら前回値を返す
- **ループ検出**：1フレーム内の再 Measure 回数に上限（3回程度）。
  超えたら警告。レイアウトのバグは無言でフレームレートを溶かす

---

## 6. 描画（最大の検討ポイント）

### 問題

既存レンダラが「描画の種類ごと」にバッチしている（DX12 の PSO 切替削減のため）。
しかし UI では Sprite と Text の描画順が要素ごとに入れ替わる。

**3Dにとって描画順は最適化の余地だが、UIにとって描画順は仕様そのもの。**
この2つを同じバッチャに通すと必ずどちらかが破綻する。

### 本命：UI専用 PSO を1つだけ作る

Sprite と Text は本質的に同じもの（テクスチャ付き色乗算クアッド）。
違いはサンプリング方法だけなので、1つのシェーダに畳み込める。

```hlsl
struct UIVertex { float2 pos; float2 uv; uint color; uint quadId; };  // 20 byte

struct UIQuadData {
  uint   texIndex;   // bindless インデックス
  uint   mode;       // 0=RGBA sprite, 1=R8 glyph, 2=SDF
  float4 clipRect;   // 軸平行クリップ矩形
};

float4 PS(VSOut i) : SV_Target {
  UIQuadData q = gQuads[i.quadId];
  Texture2D tex = ResourceDescriptorHeap[q.texIndex];   // SM6.6 bindless
  float4 c = tex.Sample(gSampler, i.uv);

  if      (q.mode == 1) c = float4(1,1,1, c.r);           // アルファのみのグリフ
  else if (q.mode == 2) c = float4(1,1,1, SdfAlpha(c.r));

  clip(any(i.screenPos < q.clipRect.xy || i.screenPos > q.clipRect.zw) ? -1 : 1);
  return c * i.color;
}
```

得られるもの：

- PSO 切替がゼロ。Sprite と Text がどう入れ替わっても同一パイプライン
- bindless によりテクスチャ切替でもバッチが切れない（アトラス統合が不要になる）
- 軸平行クリップがピクセルシェーダで処理でき、**ステンシル不要**。
  回転クリップのときだけステンシルへフォールバック
- 結果として UI 全体がドローコール 1〜数本に収まる

### 次善：既存レンダラを使う場合の重なり判定つき再ソート

順序を守る必要があるのは**重なっている要素だけ**という観察を使う。

```cpp
for (auto& item : drawList) {
  for (int b = openBatches.size()-1; b >= 0; --b) {
    if (!Compatible(openBatches[b], item)) continue;          // PSO/テクスチャ一致
    if (OverlapsAnyBetween(item.aabb, b, drawList)) break;    // 間の要素と重なる→合流不可
    openBatches[b].Add(item);
    goto merged;
  }
  openBatches.push_back(NewBatch(item));
merged:;
}
```

- 探索は直近 8 バッチ程度で打ち切る
- 重なり判定は AABB で十分。保守的に「重なる」と判定する分には安全
- Unity UGUI と同じ方式。リスト状 UI には劇的に効き、密な UI では効きにくい

### 却下案

**深度バッファによる順序解決**は不可。UI はほぼ全面がアルファブレンドのため、
深度テストでは正しい合成にならない。

### 進め方

1. まず順序を完全に守る素直な実装（要素ごとに 1 ドロー）で動かす
2. 「バッチが切れた理由の内訳」計測を入れる
3. その後で単一 PSO 化へ移行する

単一 PSO 化は DrawList 構築部分だけの変更で済み、Element 側の設計に影響しない。

### バッチ構造

```cpp
struct Batch {
  TextureId tex; BlendMode blend; Rect scissor;
  uint32_t indexOffset, indexCount;
};
```

バッチが切れる条件：テクスチャ / ブレンド / クリップ矩形 / シェーダ の変化。
頂点バッファはリングバッファ（3フレーム分）に積む。

---

## 7. クリッピング

```
ClipScope開始 → ステンシル書き込み描画 (ref = depth+1)
              → 中身を StencilFunc(Equal, ref) で描画
ClipScope終了 → 書き込んだ領域を復元 (decrement)
```

- **Zオーダーソートはクリップスコープをまたげない**。
  DrawList はスコープ単位のツリーとして持ち、ソートはスコープ内で行う
- ネストは 8 段程度に制限（超えたら警告）
- 高速パス：回転・歪みのない矩形クリップは Scissor またはPSシェーダの clip へ
- レンダーターゲットに D24S8 を要求することをエンジン仕様として明記

---

## 8. 入力（キーマウ）

```
Hit-Test  : 最前面から後方へ。world 逆行列でローカル座標化 → 矩形/アルファ判定
            クリップスコープ外は即除外
Hover     : 毎フレーム最前面の1つだけ。Enter/Leave を発行
Capture   : Down した要素が Up まで入力を独占（ドラッグ・スライダの必須要件）
Focus     : キー入力の宛先。Tab 移動順、フォーカスリング表示
Modal     : 入力ルートをスタック管理。最上位より下はヒットテスト対象外
```

イベント伝播は **Tunnel（親→子、`PreviewXxx`）→ Target → Bubble（子→親）** の3段。
`handled` で停止。ウィンドウのドラッグ移動は
「タイトルバーの子がクリックを消費しなければ親が拾う」という Bubble で自然に書ける。

イベント種別：
`PointerEnter / Leave / Down / Up / Click / DoubleClick / Wheel / ContextMenu /
 DragBegin / Drag / DragEnd / KeyDown / KeyUp / Char / FocusIn / FocusOut`

キーマウ前提なら、ホイールスクロール・右クリックメニュー・IME 経由の `Char`
を最初から組み込んでおく。

---

## 9. アニメーション（タイムライン）

```cpp
struct Keyframe { float time; Value value; Easing ease; };

struct Track {
  ElementPath target;    // "Panel/Icon" — 再生開始時に一度だけ解決
  PropertyId  prop;      // プロパティ反射のID
  std::vector<Keyframe> keys;
};

struct Clip {
  float duration;
  std::vector<Track>  tracks;
  std::vector<Marker> events;   // 時刻付きコールバック（SE、遷移完了通知）
  LoopMode loop;
};
```

- パス解決は毎フレームやらない。再生開始時に `ElementHandle` へ解決してキャッシュ
- Player に `layer` と `weight` を持たせ、同一プロパティの競合を加重合成
- キーフレーム検索は前フレームのインデックスから線形探索
- Marker イベントは「フェードイン完了後にボタンを有効化」等の演出制御に必須

---

## 10. シリアライズとエディタ

```json
{
  "guid": "a3f1-…",
  "name": "OkButton",
  "anchor": { "min": [0.5, 1.0], "max": [0.5, 1.0] },
  "rect":   { "offset": [0, -40], "size": [160, 48] },
  "drawables": [
    { "type": "Sprite", "atlas": "ui.atlas", "key": "btn_bg", "slice": [8,8,8,8] },
    { "type": "Text",   "font": "main", "size": 20, "text": "@ui.ok" }
  ],
  "behaviors": [ { "type": "Clickable", "props": { "repeat": false } } ],
  "children": []
}
```

- **GUID 必須**。名前やインデックス参照はエディタでの並べ替えで壊れる
- プレハブのネストと差分オーバーライド：
  インスタンス側は `{ "prefab": "guid", "overrides": { "OkButton/Text.text": "はい" } }` のみ保持
- テキストは直書きせず `@ui.ok` のキーにしてローカライズテーブル経由で解決
- **スキーマバージョン `{"version": 3, ...}` とマイグレーション関数の連鎖**を最初から入れる
  （これだけは後から追加できない）
- ホットリロードは「JSON再読込 → ツリー再構築 → 実行時状態（スクロール位置・フォーカス）復元」まで含める

---

## 11. ウィンドウマネージャ

```
WindowLayer (Element)          … 子の並び順 = Zオーダー
 └ Window (Element)
    ├ [Draggable] [Resizable] [Focusable]
    ├ Frame      (Sprite 9スライス)
    ├ TitleBar   (Element) ── Title(Text), CloseButton
    └ Content    (Element) ── [ClipContent] 中身は自由
```

`WindowLayer` の仕事は**フォーカスされた子を配列末尾へ移す**だけ。

- リサイズグリップは 8 方向。判定はウィンドウ矩形の内外にまたがる帯
  （外側4px / 内側4px 程度）を取り、中身より優先してヒットさせる
- `minSize` 必須。加えてタイトルバーが画面外に出ない拘束（出ると掴めなくなる）
- カーソル形状変更 API（`↔ ↕ ⤡`）が必要。既存エンジンに無ければ追加
- クリックスルー：非アクティブウィンドウのクリックで、
  前面化と同時に中のボタンも押される挙動を推奨（ゲームUIでは快適）
- ウィンドウの位置・サイズを GUID キーで設定ファイルに保存／復元

---

## 12. HUD 連携（イベント通知）

```cpp
template<class... A> class Signal {
public:
  [[nodiscard]] ScopedConnection Connect(std::function<void(A...)>);
  void Emit(A...);
};

class HpBar : public IBehavior {
  ScopedConnection conn_;
  void OnAttach(Element& e) override {
    conn_ = player.onHpChanged.Connect([&](int hp, int max){
      pendingHp_ = { hp, max };
      e.MarkDirty(DirtyFlags::Content);   // 即座に反映しない
    });
  }
};
```

必ず踏む地雷3つ：

1. **ダングリング** — `ScopedConnection` を Behavior のメンバに持たせ RAII で自動切断
2. **フレーム中の再入** — 通知は「値を控えて dirty を立てるだけ」。実処理は UI 更新フェーズへ遅延
3. **通知の洪水** — dirty フラグ方式なら 1 フレーム 1 回に自動集約される
   （ポーリングより速くなる理由）

---

## 13. スクロールビューと仮想化

```
ScrollView (Element)
 ├ Viewport [ClipContent]     … 見える窓
 │   └ Content               … 全体。これを動かす
 └ ScrollBar [Draggable]
```

スクロール量は Content の位置オフセット。これは `Transform` dirty だけなので
**レイアウト再計算が走らない**。スクロールが重い実装の大半はここで Layout を汚している。

```cpp
class ItemsView : public IBehavior {
  std::function<int()>               count;   // データ件数
  std::function<void(Element&, int)> bind;    // 内容の流し込み
  std::vector<Element*> pool_;                // 使い回す実体
  int firstVisible_, lastVisible_;
};
```

- 可視範囲＋前後マージン（各2〜3行）ぶんの Element だけ生成し、リサイクルして `bind` を呼ぶ
- 前提として **index → Y座標が O(1)** で求まること。
  可変高なら累積和（Fenwick木）か、推定値で並べて実測後に補正
- データ10万件でも実在 Element は数十個

---

## 14. メモリ管理

| 対象 | 方針 |
|---|---|
| Element本体 | 固定サイズなのでチャンク型スロット配列。世代番号を持たせ Handle の実体にする |
| Behavior | 型ごとのプール。個別 `new` を避ける |
| 子リスト | `vector<Handle>` |
| DrawList・頂点・Measure中間結果 | **フレームアロケータ**（線形確保、毎フレーム巻き戻すだけ） |
| 名前文字列 | 開発ビルドのみ保持。リリースではハッシュ値のみ |

Element 1つ 150〜200 byte、1万個で 2MB 弱。
切り詰めるより、フレームアロケータで毎フレームの malloc/free を消す方が効果が大きい。

---

## 15. デバッグ機能

- **ランタイムインスペクタ**：ツリー表示 → 要素選択 → プロパティ編集。
  エディタとまったく同じコードが動く（プロパティ反射の投資が回収される）
- **オーバーレイ**：矩形 / pivot / anchor の可視化、クリップ境界、
  ヒットした要素、現在のフォーカス
- **計測パネル**：総Element数 / 可視数、Measure呼び出し回数、DrawCall数、頂点数、UI更新時間
- 最も効くのは **「バッチが切れた理由の内訳」**
  （テクスチャ変更 n回 / クリップ変更 n回 / ブレンド変更 n回）。
  内訳がないと打ち手が決まらない

**このデバッグUI自体を、作っているUIシステムで実装すること。**
最初のヘビーユーザーが自分になり、使いにくい部分が即座に露呈する。

---

## 16. 効率と使いやすさの両立方針

原則は**「表層のAPI」と「内部表現」を切り離す**こと。

```
表層： element->SetPosition(...)  親子関係、Behavior、直感的
  ↓  1フレームに1回だけ変換
内部： DrawList（平坦な配列）、dirty な部分木のみ再計算
```

| 後付け不可能（最初に入れる） | 後から入る（今は素朴でOK） |
|---|---|
| Handle間接参照 `{index, generation}` | プールアロケータ |
| Dirtyフラグによる差分更新 | SoA化 |
| DrawListの平坦化 | 仮想関数の除去 |
| シリアライズのスキーマバージョン | |

`IDrawable::Emit` の仮想呼び出しコストは気にしなくてよい。
UI要素は数千オーダーで、実際のボトルネックは常に
**ドローコール数**と**不要なレイアウト再計算**の2つ。

---

## 17. ライブラリ化について

### 依存を「3つの穴」に閉じ込める

UIがエンジンを呼ぶのではなく、**エンジンがUIの求めるインターフェースを実装する**形に反転させる。

```cpp
// ① 描画バックエンド
class IRenderBackend {
  virtual void Render(const DrawList&) = 0;
  virtual TextureId CreateTexture(const uint8_t* rgba, int w, int h) = 0;
  virtual void DestroyTexture(TextureId) = 0;
};

// ② テキスト計測（本当の難所。レイアウトが必ず要求する）
class ITextMeasure {
  virtual Vec2 Measure(FontId, std::string_view, float maxWidth) = 0;
  virtual void Shape(FontId, std::string_view, std::vector<Glyph>& out) = 0;
};

// ③ アセット解決
class IAssetResolver {
  virtual TextureId ResolveTexture(std::string_view key, Rect& outUV) = 0;
  virtual FontId    ResolveFont(std::string_view key) = 0;
};
```

### ターゲット分割

```
ui-core      … Element, Layout, Input, Animation, DrawList   （依存ゼロ）
ui-widgets   … Button, ScrollView, Window, WindowManager     （coreのみ依存）
ui-serialize … JSON入出力、プレハブ                          （任意）
ui-debug     … インスペクタ、計測オーバーレイ                （開発ビルドのみ）
```

`ui-core` を標準ライブラリ以外に依存させないことが最重要。

### ホスト環境への配慮

- **アロケータ注入** `SetAllocator(alloc, free, userdata)`（後付けが極めて困難）
- **例外・RTTI を使わない**。`GetBehavior<T>()` は `dynamic_cast` ではなく型IDの比較で実装
- **数学型の橋渡し**。ImGui の `IM_VEC2_CLASS_EXTRA` 相当のマクロを用意
- **グローバル状態を持たない**。`UIContext` を明示的に受け渡す
- **配布はソース形式**（static lib / CMake サブディレクトリ）。DLL境界を跨がせない

### 副産物：ヘッドレステスト

```cpp
TEST(Layout, AnchorStretch) {
  UIContext ctx(&nullBackend, &fakeTextMeasure);
  auto* root  = ctx.CreateRoot(1920, 1080);
  auto* child = root->CreateChild();
  child->SetAnchor({0,0}, {1,1}); child->SetOffset(10,10,10,10);
  ctx.Update(0.016f);
  EXPECT_EQ(child->WorldRect(), Rect(10,10,1900,1060));
}
```

DrawList をゴールデンデータと突き合わせる回帰テストも書ける。

### 推奨する進め方

**今すぐ切り出すのは早い。** 利用者が1人の段階で汎用APIを設計すると
不要な抽象化が積み上がる。

1. `ui-core` 相当のディレクトリを切り、
   **そこから既存エンジンのヘッダを一切 include しない**ルールだけ課す
2. 依存が必要になったら、その都度インターフェースを1つ追加して外へ出す
3. ゲームを1本作り切って、API が実戦で削られるのを待つ
4. その後で物理的に切り出す

include 規約を機械的にチェックするスクリプトを最初に仕込むとルールが自然に守られる。

---

## 付録：ローカルセッションで最初に確認すべきこと

### A. レンダラの調査（最優先）

| 確認項目 | 満たさない場合の影響 |
|---|---|
| 描画順を呼び出し順で完全制御できるか（内部で勝手にソートしないか） | 🔴 Zオーダーもクリップも成立しない |
| ステンシル状態を外部指定できるか | 🔴 クリッピング不可 |
| 任意の2D行列を渡せるか（親からの合成変換） | 🔴 階層構造が成立しない |
| 頂点カラー乗算があるか | 🟡 opacity伝播・Hover時のtintが不可 |
| 9スライス対応 | 🟡 可変サイズのウィンドウ枠が作れない |
| Text: 折返し・行間・アライン | 🟡 自動レイアウトと噛み合わない |
| Text: 文字単位の頂点/矩形取得 | 🟢 タイプライタ演出、文字ごとのアニメ |

具体的に読むべき箇所：

- バッチャ本体（種類ごとのソート/マージのロジック）
- PSO の生成・キャッシュ機構、ルートシグネチャの構成
- ディスクリプタヒープの管理方法（bindless に移行可能か）
- `Sprite` / `Text` の公開インターフェースと内部の頂点生成
- レンダーターゲットのフォーマット（D24S8 があるか）
- 定数バッファ / StructuredBuffer の更新経路

### B. 判断したい論点

1. 単一PSO + bindless 構成へ移行可能か。移行コストはどれくらいか
2. 移行しない場合、既存バッチャに「順序保証パス」を追加できるか
3. `Text` の Measure / Shape をレイアウトから同期呼び出しできるか
   （非同期・遅延生成だとレイアウトが1フレーム遅れる）
4. `ui-core` を切り出すとして、既存の数学型・アセット管理との結合度はどの程度か

### C. 実装の着手順（案）

```
1. Element ツリー + Handle + コマンドキュー
2. アンカーレイアウト + Transform伝播
3. DrawList 構築 → 素直な1要素1ドローで画面に出す   ← ここで一度動くものにする
4. 入力（HitTest / Hover / Capture / Focus / 伝播）
5. プロパティ反射 + デバッグインスペクタ            ← 以降の開発速度が変わる
6. Behavior 群（Clickable / Draggable / VisualState）
7. ClipContent + ScrollView
8. Window / WindowLayer
9. 計測パネル → バッチ最適化（単一PSO化）
10. アニメーション
11. JSON シリアライズ
12. 自動レイアウト（Flex / Grid）
13. 仮想化 ItemsView
```

3 の段階で必ず一度「画面に出る」状態を作ること。
そこまでを最短で通してから肉付けする。
