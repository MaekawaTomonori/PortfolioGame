#include "KeyGuide.hpp"

#include <fstream>

#include "imgui.h"
#include "Log.hpp"
#include "Utils.hpp"
#include "Input.hpp"
#include "externals/json/json.hpp"
#include "Pattern/Singleton.hpp"

void KeyGuide::Initialize() {
    input_ = Singleton<Input>::GetInstance();

    frame_ = std::make_unique<Sprite>();
    frame_->Initialize("white_x16.png");
    frame_->SetPosition(position_);
    frame_->SetColor({0.3f, 0.3f, 0.3f, 0.8f});
    Load();
}

void KeyGuide::Update() {
    if (reload_) {
        keys_.clear();
        Load();
        reload_ = false;
    }

    frame_->SetPosition(position_);
    frame_->Update();

    for (const auto& [key ,value]: keys_) {
        value.frame->SetColor(value.defaultColor);
        if (input_->IsPress(key)) {
            value.frame->SetColor(value.pressColor);
        }

        if (value.frame) {
            value.frame->SetPosition(position_ + value.position);
            value.frame->Update();
        }
        if (value.key) {
            value.key->SetPosition(position_ + value.position);
            value.key->Update();
        }
    }
}

void KeyGuide::Draw() const {
    if (frame_) {
        frame_->Draw();
    }

    for (const auto& key : keys_ | std::views::values) {
        if (key.frame) {
            key.frame->Draw();
        }
        if (key.key) {
            key.key->Draw();
        }
    }
}

void KeyGuide::Debug() {
    ImGui::Begin("KeyGuide");

    ImGui::Text("Global Settings");
    ImGui::Separator();

    // 全体位置の編集
    ImGui::Text("Position");
    if (ImGui::DragFloat2("##Position", &position_.x, 1.0f, 0.0f, 1920.0f)) {
        if (frame_) {
            frame_->SetPosition(position_);
        }
    }

    // フレームサイズの編集
    if (frame_) {
        ImGui::Text("Frame Size");
        Vector2 frameSize = frame_->GetSize();
        if (ImGui::DragFloat2("##FrameSize", &frameSize.x, 1.0f, 10.0f, 500.0f)) {
            frame_->SetSize(frameSize);
        }

        // フレーム色の編集
        ImGui::Text("Frame Color");
        Vector4 frameColor = frame_->GetColor();
        if (ImGui::ColorEdit4("##FrameColor", &frameColor.x)) {
            frame_->SetColor(frameColor);
        }
    }

    ImGui::Separator();

    // キー追加UI
    ImGui::Text("Add New Key");

    // キーコード選択（ComboBox）
    const char* previewKey = KeyCodeMap[selectedKeyIndex_].name;
    if (ImGui::BeginCombo("Key Code", previewKey)) {
        for (int i = 0; i < std::size(KeyCodeMap); i++) {
            const bool isSelected = (selectedKeyIndex_ == i);
            if (ImGui::Selectable(KeyCodeMap[i].name, isSelected)) {
                selectedKeyIndex_ = i;
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // 選択されたキーコードを表示
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(0x%02X)", KeyCodeMap[selectedKeyIndex_].code);

    // テクスチャ選択
    ImGui::Text("Texture");
    ImGui::InputText("##Texture", newKeyTexture_, sizeof(newKeyTexture_));
    ImGui::SameLine();
    if (ImGui::Button("...##BrowseTexture")) {
        std::string selectedFile = Utils::OpenFileDialog("PNG");
        if (!selectedFile.empty()) {
            // Assets/Resources/からの相対パスに変換
            std::filesystem::path fullPath(selectedFile);
            std::string filename = fullPath.filename().string();
            strcpy_s(newKeyTexture_, sizeof(newKeyTexture_), filename.c_str());
        }
    }

    // 現在のテクスチャ名を表示
    if (strlen(newKeyTexture_) > 0) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "[%s]", newKeyTexture_);
    }

    // クリップボードから貼り付けボタン
    if (hasClipboard_) {
        if (ImGui::Button("Paste from Clipboard")) {
            strcpy_s(newKeyTexture_, sizeof(newKeyTexture_), clipboardKey_.texture.c_str());
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.5f, 1.0f), "(Clipboard available)");
    }

    if (ImGui::Button("Add Key")) {
        BYTE keyCode = KeyCodeMap[selectedKeyIndex_].code;
        if (!keys_.contains(keyCode)) {
            Key newKey;
            newKey.texture = newKeyTexture_;

            // クリップボードからデータを適用
            if (hasClipboard_) {
                newKey.position = clipboardKey_.position;
                newKey.defaultColor = clipboardKey_.defaultColor;
                newKey.pressColor = clipboardKey_.pressColor;
            }

            // キーを登録
            newKey.frame = std::make_unique<Sprite>();
            newKey.frame->Initialize("white_x16.png");
            newKey.frame->SetPosition(position_ + newKey.position);
            newKey.frame->SetSize(hasClipboard_ ? clipboardKey_.frameSize : Vector2{100.0f, 100.0f});
            newKey.frame->SetColor(newKey.defaultColor);

            newKey.key = std::make_unique<Sprite>();
            newKey.key->Initialize(newKey.texture);
            newKey.key->SetPosition(position_ + newKey.position);
            newKey.key->SetSize(hasClipboard_ ? clipboardKey_.labelSize : Vector2{80.0f, 80.0f});

            keys_[keyCode] = std::move(newKey);

            Log::Send(Log::Level::INFO, "KeyGuide: Added key " + std::to_string(keyCode));
        } else {
            Log::Send(Log::Level::WARNING, "KeyGuide: Key code " + std::to_string(keyCode) + " already exists");
        }
    }

    ImGui::Separator();

    // キー情報の表示と編集
    ImGui::Text("Keys: %zu", keys_.size());

    if (ImGui::TreeNode("Key Details")) {
        int index = 0;
        BYTE keyToDelete = 255; // 削除するキーコード（255は無効値）

        for (auto& [keyCode, key] : keys_) {
            ImGui::PushID(index++);

            // キーコードに対応する名前を検索
            const char* keyName = nullptr;
            for (int i = 0; i < std::size(KeyCodeMap); i++) {
                if (KeyCodeMap[i].code == keyCode) {
                    keyName = KeyCodeMap[i].name;
                    break;
                }
            }

            std::string label = keyName
                ? std::string(keyName) + " (0x" + std::to_string(keyCode) + ")"
                : "Unknown (0x" + std::to_string(keyCode) + ")";

            if (ImGui::TreeNode(label.c_str())) {
                ImGui::Text("Texture");

                // 現在のテクスチャ名を編集可能なバッファにコピー
                char textureBuffer[256];
                strcpy_s(textureBuffer, sizeof(textureBuffer), key.texture.c_str());

                bool textureChanged = false;
                if (ImGui::InputText("##EditTexture", textureBuffer, sizeof(textureBuffer))) {
                    textureChanged = true;
                }

                ImGui::SameLine();
                if (ImGui::Button("...##BrowseEditTexture")) {
                    std::string selectedFile = Utils::OpenFileDialog("PNG");
                    if (!selectedFile.empty()) {
                        std::filesystem::path fullPath(selectedFile);
                        std::string filename = fullPath.filename().string();
                        strcpy_s(textureBuffer, sizeof(textureBuffer), filename.c_str());
                        textureChanged = true;
                    }
                }

                // テクスチャが変更された場合、Spriteを再初期化
                if (textureChanged) {
                    key.texture = std::string(textureBuffer);
                    if (key.key && !key.texture.empty()) {
                        Vector2 currentSize = key.key->GetSize();
                        Vector2 currentPos = key.key->GetPosition();
                        key.key->SetTexture(key.texture);
                        key.key->SetPosition(currentPos);
                        key.key->SetSize(currentSize);
                        key.key->Update();
                    }
                }

                // 現在のテクスチャ名を表示
                if (!key.texture.empty()) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "[%s]", key.texture.c_str());
                }

                ImGui::Separator();
                
                // 相対位置（オフセット）の編集
                Vector2 offset = key.position;
                ImGui::Text("Frame Offset");
                if (ImGui::DragFloat2("##Offset", &offset.x, 1.0f, -500.0f, 500.0f)) {
                    key.position = offset;
                }

                // フレームサイズの編集
                if (key.frame) {
                    ImGui::Text("Frame Size");
                    Vector2 keyFrameSize = key.frame->GetSize();
                    if (ImGui::DragFloat2("##KeyFrameSize", &keyFrameSize.x, 1.0f, 10.0f, 500.0f)) {
                        key.frame->SetSize(keyFrameSize);
                    }
                }

                // ラベルサイズの編集
                if (key.key) {
                    ImGui::Text("Label Size");
                    Vector2 labelSize = key.key->GetSize();
                    if (ImGui::DragFloat2("##LabelSize", &labelSize.x, 1.0f, 10.0f, 500.0f)) {
                        key.key->SetSize(labelSize);
                    }
                }

                ImGui::Text("Key Color");
                if (ImGui::ColorEdit4("##KeyColor", &key.defaultColor.x)) {
                    key.frame->SetColor(key.defaultColor);
                }

                ImGui::Text("Press Color");
                ImGui::ColorEdit4("##PressColor", &key.pressColor.x);

                ImGui::Separator();

                // コピーボタン
                if (ImGui::Button("Copy This Key")) {
                    clipboardKey_.position = key.position;
                    clipboardKey_.texture = key.texture;
                    clipboardKey_.defaultColor = key.defaultColor;
                    clipboardKey_.pressColor = key.pressColor;
                    if (key.frame) {
                        clipboardKey_.frameSize = key.frame->GetSize();
                    }
                    if (key.key) {
                        clipboardKey_.labelSize = key.key->GetSize();
                    }
                    hasClipboard_ = true;
                    Log::Send(Log::Level::INFO, "KeyGuide: Copied key " + std::to_string(keyCode));
                }

                ImGui::SameLine();

                // 削除ボタン
                if (ImGui::Button("Delete This Key")) {
                    keyToDelete = keyCode;
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        if (keyToDelete != 255) {
            keys_.erase(keyToDelete);
        }

        ImGui::TreePop();
    }

    ImGui::Separator();

    // Load/Saveボタン
    if (ImGui::Button("Load")) {
        reload_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        Save();
    }

    ImGui::End();
}

void KeyGuide::Load() {
    std::ifstream file(JsonPath);
    if (!file.is_open()) {
        Log::Send(Log::Level::WARNING, "KeyGuide: JSON file not found - " + JsonPath);
        return;
    }

    nlohmann::json root;
    try {
        file >> root;
        file.close();
    } catch (const nlohmann::json::parse_error& e) {
        Log::Send(Log::Level::ERR, "KeyGuide: JSON parse error - " + std::string(e.what()));
        file.close();
        return;
    }

    // Settingsから基本情報を読み込み
    if (root.contains("Settings") && root["Settings"].is_object()) {
        auto& settings = root["Settings"];

        if (settings.contains("Position") && settings["Position"].is_array() && settings["Position"].size() >= 2) {
            position_ = {settings["Position"][0].get<float>(), settings["Position"][1].get<float>()};
        }

        Vector2 frameSize = {100.0f, 100.0f};
        if (settings.contains("Size") && settings["Size"].is_array() && settings["Size"].size() >= 2) {
            frameSize = {settings["Size"][0].get<float>(), settings["Size"][1].get<float>()};
        }

        Vector4 frameColor = {0.3f, 0.3f, 0.3f, 0.8f};
        if (settings.contains("Color") && settings["Color"].is_array() && settings["Color"].size() >= 4) {
            frameColor = {
                settings["Color"][0].get<float>(),
                settings["Color"][1].get<float>(),
                settings["Color"][2].get<float>(),
                settings["Color"][3].get<float>()
            };
        }

        // 全体フレームの設定を更新
        if (frame_) {
            frame_->SetPosition(position_);
            frame_->SetSize(frameSize);
            frame_->SetColor(frameColor);
        }
    }

    // Keys配列を読み込み
    if (root.contains("Keys") && root["Keys"].is_array()) {
        keys_.clear(); // 既存のキーをクリア

        for (const auto& keyData : root["Keys"]) {
            Key newKey;

            // キーコードを取得
            BYTE keyCode = static_cast<BYTE>(keyData.value("Key", 0));

            // フレームオフセットを取得
            Vector2 frameOffset = {0.0f, 0.0f};
            if (keyData.contains("FrameOffset") && keyData["FrameOffset"].is_array() && keyData["FrameOffset"].size() >= 2) {
                frameOffset = {
                    keyData["FrameOffset"][0].get<float>(),
                    keyData["FrameOffset"][1].get<float>()
                };
            }

            // フレームサイズを取得
            Vector2 keyFrameSize = {100.0f, 100.0f};
            if (keyData.contains("FrameSize") && keyData["FrameSize"].is_array() && keyData["FrameSize"].size() >= 2) {
                keyFrameSize = {
                    keyData["FrameSize"][0].get<float>(),
                    keyData["FrameSize"][1].get<float>()
                };
            }

            // ラベルサイズを取得
            Vector2 labelSize = {80.0f, 80.0f};
            if (keyData.contains("LabelSize") && keyData["LabelSize"].is_array() && keyData["LabelSize"].size() >= 2) {
                labelSize = {
                    keyData["LabelSize"][0].get<float>(),
                    keyData["LabelSize"][1].get<float>()
                };
            }

            // テクスチャパスを取得
            std::string texturePath = keyData.value("Texture", "white_x16.png");

            // Color
            newKey.defaultColor = {};
            newKey.pressColor = {};

            if (keyData.contains("DefaultColor") && keyData["DefaultColor"].is_array() && keyData["DefaultColor"].size() >= 4) {
                newKey.defaultColor = {
                    keyData["DefaultColor"][0].get<float>(),
                    keyData["DefaultColor"][1].get<float>(),
                    keyData["DefaultColor"][2].get<float>(),
                    keyData["DefaultColor"][3].get<float>()
                };
            }

            if (keyData.contains("PressColor") && keyData["PressColor"].is_array() && keyData["PressColor"].size() >= 4) {
                newKey.pressColor = {
                    keyData["PressColor"][0].get<float>(),
                    keyData["PressColor"][1].get<float>(),
                    keyData["PressColor"][2].get<float>(),
                    keyData["PressColor"][3].get<float>()
                };
            }

            // 位置を保存（オフセット）
            newKey.position = frameOffset;

            // フレームスプライトを作成
            newKey.frame = std::make_unique<Sprite>();
            newKey.frame->Initialize("white_x16.png");
            newKey.frame->SetPosition(position_ + newKey.position);
            newKey.frame->SetSize(keyFrameSize);
            if (frame_) {
                newKey.frame->SetColor(frame_->GetColor());
            }

            // キースプライトを作成
            if (!texturePath.empty()) {
                newKey.texture = texturePath;
                newKey.key = std::make_unique<Sprite>();
                newKey.key->Initialize(texturePath);
                newKey.key->SetPosition(position_ + newKey.position);
                newKey.key->SetSize(labelSize);
            }

            // マップに追加
            keys_[keyCode] = std::move(newKey);
        }
    }

    Log::Send(Log::Level::INFO, "KeyGuide: Loaded " + JsonPath);
}

void KeyGuide::Save() {
    nlohmann::json root;

    // Settingsを保存
    nlohmann::json settings;
    settings["Position"] = {position_.x, position_.y};

    if (frame_) {
        auto size = frame_->GetSize();
        auto color = frame_->GetColor();
        settings["Size"] = {size.x, size.y};
        settings["Color"] = {color.x, color.y, color.z, color.w};
    }

    root["Settings"] = settings;

    // Keys配列を構築
    nlohmann::json keysArray = nlohmann::json::array();
    for (const auto& [keyCode, key] : keys_) {
        nlohmann::json keyJson;
        keyJson["Key"] = keyCode;

        keyJson["Texture"] = key.texture;

        keyJson["FrameOffset"] = {key.position.x, key.position.y};

        if (key.frame) {
            auto frameSize = key.frame->GetSize();
            keyJson["FrameSize"] = {frameSize.x, frameSize.y};
        }

        if (key.key) {
            auto labelSize = key.key->GetSize();
            keyJson["LabelSize"] = {labelSize.x, labelSize.y};

        }

        keyJson["DefaultColor"] = { key.defaultColor.x, key.defaultColor.y, key.defaultColor.z, key.defaultColor.w };
        keyJson["PressColor"] = { key.pressColor.x, key.pressColor.y, key.pressColor.z, key.pressColor.w };

        keysArray.push_back(keyJson);
    }
    root["Keys"] = keysArray;

    // ファイルに書き込み
    std::ofstream file(JsonPath, std::ios::trunc);
    if (!file.is_open()) {
        Log::Send(Log::Level::ERR, "KeyGuide: Failed to open file for write - " + JsonPath);
        return;
    }

    file << root.dump(4) << '\n';
    file.close();

    Log::Send(Log::Level::INFO, "KeyGuide: Saved " + JsonPath);
}

void KeyGuide::Register(const BYTE& _code, Key _key) {
    _key.frame = std::make_unique<Sprite>();
    _key.frame->Initialize("white_x16.png");
    _key.frame->SetPosition(position_ + _key.position);
    _key.frame->SetSize({100.0f, 100.0f});
    _key.frame->SetColor(_key.defaultColor);

    _key.key = std::make_unique<Sprite>();
    _key.key->Initialize(_key.texture);
    _key.key->SetPosition(position_ + _key.position);
    _key.key->SetSize({80.f, 80.f});

    keys_[_code] = std::move(_key);
}


