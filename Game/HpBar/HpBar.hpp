#ifndef HpBar_HPP_
#define HpBar_HPP_
#include <memory>

#include "Sprite.hpp"
#include "Math/Vector3.hpp"

/** @brief 持ち主の頭上に画面空間で表示するHPバー
 * 3Dモデルではなくスプライトで描くため、ライティングや影の影響を受けない。
 * 満タンの間は表示せず、ダメージを受けて初めて現れる。
 * 見た目の設定は全HPバー共通で、Assets/Data/EnemyHpBar/EnemyHpBar.json から読み込む。
 */
class HpBar {
public:
    /** @brief 全HPバー共通の見た目の設定 */
    struct Settings {
        // 基準解像度(1280x720)でのピクセル。実際の大きさは画面の倍率に合わせて拡縮される
        Vector2 size = {40.f, 6.f};
        float scale = 1.f;
        // 持ち主の原点からの高さ（ワールド）と、画面上でさらに上へずらす量（基準解像度のピクセル）
        float worldOffsetY = 1.f;
        float screenOffsetY = 24.f;
        Vector4 backColor  = {0.35f, 0.f, 0.f, 0.8f};
        Vector4 trailColor = {1.f, 0.9f, 0.1f, 1.f};
        Vector4 frontColor = {0.2f, 0.9f, 0.2f, 1.f};
        // 被弾後、残像が減り始めるまでの待ち時間と、減る速さ（割合/秒）
        float trailDelay = 0.25f;
        float trailSpeed = 1.5f;
    };

private:
    static inline Settings settings_{};

    const Vector3* ownerPosition_ = nullptr;

    std::unique_ptr<Sprite> back_;
    std::unique_ptr<Sprite> trail_;
    std::unique_ptr<Sprite> front_;

    float ratio_ = 1.f;
    // 減った分を少し遅れて追いかける残像の割合
    float trailRatio_ = 1.f;
    float trailDelay_ = 0.f;

    bool visible_ = false;

public:
    void Initialize(const Vector3* _ownerPos);
    void Update(float _hpRatio);
    void Draw() const;

    /** @brief 共通設定をJSONから読み込む（無い項目は既定値のまま） */
    static void LoadSettings();

    /** @brief 共通設定をJSONへ書き出す */
    static void SaveSettings();

    /** @brief 共通設定を調整するImGuiウィンドウ */
    static void DebugSettings();

private:
    void UpdateTrail(float _deltaTime);

    /** @brief 頭上の位置をスクリーン座標へ変換する
     * @param _screenPos 出力先（ピクセル）
     * @return カメラの前方にあり、変換できたか
     */
    bool ProjectToScreen(Vector2& _screenPos) const;

}; // class HpBar

#endif // HpBar_HPP_
