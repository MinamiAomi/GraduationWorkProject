#include "BatteryParticles.h"

#include "Engine/Framework/AssetManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Trolley.h"



void BatteryParticles::Initialize(const Transform* transform, BatsManager* batsManager)
{
    transform_.SetParent(transform, false);
    auto assetManager = AssetManager::GetInstance();
    model_ = assetManager->modelMap.Get("Box")->Get();
    radius_ = Trolley::GetInstance()->GetBatteryRadius();
    emitTimer_ = 0;
    minScale_ = 0.1f;
    maxScale_ = 0.2f;
    material_ = std::make_shared<Material>();
    material_->emissive = { 1.0f,1.0f,1.0f };
    material_->emissiveIntensity = 10.0f;

    toBatMaterial_ = std::make_shared<Material>();
    toBatMaterial_->emissive = { 1.0f,1.0f,1.0f };
    toBatMaterial_->emissiveIntensity = 10.0f;
    toBatMaterial_->albedo = Vector3{ 0.75f,0.2f,0.75f };

    batsManager_ = batsManager;

    JSON_OPEN("Resources/Data/GameScene/batteryParticles.json");
    JSON_OBJECT("batteryParticles");
    JSON_LOAD_BY_NAME("minSpeed_", minSpeed_);
    JSON_LOAD_BY_NAME("maxSpeed_", maxSpeed_);
    JSON_LOAD_BY_NAME("minAngularVelocity_", minAngularVelocity_);
    JSON_LOAD_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
    JSON_LOAD_BY_NAME("scaleDecay_", scaleDecay_);
    JSON_LOAD_BY_NAME("emitInterval_", emitInterval_);
    JSON_LOAD_BY_NAME("minScale_", minScale_);
    JSON_LOAD_BY_NAME("maxScale_", maxScale_);
    JSON_LOAD_BY_NAME("color_", color_);
    JSON_LOAD_BY_NAME("toBatSpeed_", toBatSpeed_);
    JSON_LOAD_BY_NAME("toBatScaleSpeed_", toBatScaleSpeed_);
    JSON_CLOSE();
    material_->albedo = color_;
}

void BatteryParticles::Update()
{
    material_->albedo = color_;
    transform_.UpdateMatrix();
    auto inverseParentMatrix = transform_.GetParent()->worldMatrix.Inverse();
    if (Trolley::GetInstance()->GetIsHitFlashlight()) {
        emitTimer_++;
        if (emitTimer_ >= emitInterval_) {
            Emit();
            emitTimer_ = 0;
        }
    }

    if (batEmitCycle_++ >= batEmitInterval) {
        batEmitCycle_ = 0;
        for (auto& bats : batsManager_->GetBatsGroups()) {
            for (auto& bat : bats->GetBats()) {
                if (!bat->isDead_) {
                    ToBatEmit(bat);
                }
            }
        }
    }


    for (auto it = particles_.begin(); it != particles_.end(); ) {

        Particle* p = it->get();
        bool isDead = false;

        switch (p->type_)
        {
        case ParticleType::ToCharge:
        {


            p->transform_.scale.x -= p->scaleSpeed_;
            p->transform_.scale.y -= p->scaleSpeed_;
            p->transform_.scale.z -= p->scaleSpeed_;
            p->transform_.translate = p->transform_.translate + p->velocity_;

            isDead = p->transform_.scale.x <= 0.0f;
            break;
        }
        case ParticleType::ToBat:
        {
            auto bat = p->targetBat_.lock();
            if (bat && !bat->isDead_) {
                auto parentLocalBat = bat->transform_.worldMatrix.GetTranslate() * inverseParentMatrix;
                Vector3 toBat = (parentLocalBat - p->transform_.translate).Normalized();
                p->velocity_ += toBat * toBatSpeed_;
                p->velocity_ = p->velocity_.Normalized() * std::clamp(p->velocity_.Length(), 0.0f, 0.2f);
                p->transform_.translate = p->transform_.translate + p->velocity_;

                float current = (p->transform_.worldMatrix.GetTranslate() - bat->transform_.worldMatrix.GetTranslate() ).Length();
                float parent = (p->transform_.GetParent()->worldMatrix.GetTranslate() - bat->transform_.worldMatrix.GetTranslate()).Length();

                float t = 1.0f - (current / parent);
                t = 1.0f - std::pow(1.0f - t, 50.0f);
                p->modelInstance_.GetMaterials()[0]->albedo.x = Math::Lerp(t, 0.46f, toBatMaterial_->albedo.x);
                p->modelInstance_.GetMaterials()[0]->albedo.y = Math::Lerp(t, 0.94f, toBatMaterial_->albedo.y);
                p->modelInstance_.GetMaterials()[0]->albedo.z = Math::Lerp(t, 1.0f, toBatMaterial_->albedo.z);


                p->transform_.scale.x += p->scaleSpeed_;
                p->transform_.scale.y += p->scaleSpeed_;
                p->transform_.scale.z += p->scaleSpeed_;
                p->transform_.scale.x = std::clamp(p->transform_.scale.x, 0.0f, 0.02f);
                p->transform_.scale.y = std::clamp(p->transform_.scale.y, 0.0f, 0.02f);
                p->transform_.scale.z = std::clamp(p->transform_.scale.z, 0.0f, 0.02f);



                isDead = (bat->transform_.worldMatrix.GetTranslate() - p->transform_.worldMatrix.GetTranslate()).Length() < 5.0f;
            }
            else {
                // ターゲットのコウモリがいなくなったら消す
                isDead = true;
            }


            break;
        }
        break;
        default:
            break;
        }


        Quaternion deltaRot = Quaternion::MakeFromEulerAngle(p->angularVelocity_);
        p->transform_.rotate = p->transform_.rotate * deltaRot;
        p->transform_.rotate = p->transform_.rotate.Normalized();


        if (isDead) {
            it = particles_.erase(it);
        }
        else {
            // 行列更新
            p->transform_.UpdateMatrix();
            p->modelInstance_.SetWorldMatrix(p->transform_.worldMatrix);

            ++it;
        }
    }
}

void BatteryParticles::DebugDraw()
{
    auto& lineDrawer = RenderManager::GetInstance()->GetLineDrawer();


    lineDrawer.DrawSphere(transform_.worldMatrix.GetTranslate(), radius_, Vector4{ material_->albedo.x,material_->albedo.y,material_->albedo.z,1.0f });

}

void BatteryParticles::Debug()
{
    ImGui::Begin("GameScene", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::TreeNode("BatteryParticles")) {

        ImGui::DragFloat("minSpeed_", &minSpeed_, 0.000000001f, 0.0f, 1.0f, "%.10f");
        ImGui::DragFloat("maxSpeed_", &maxSpeed_, 0.000000001f, minSpeed_, 1.0f, "%.10f");
        if (minSpeed_ > maxSpeed_) {
            maxSpeed_ = minSpeed_ + 0.00001f;
        }
        ImGui::DragFloat3("minAngularVelocity_", &minAngularVelocity_.x, 0.0000001f, 0.0f, 1.0f, "%.10f");
        ImGui::DragFloat3("maxAngularVelocity_", &maxAngularVelocity_.x, 0.0000001f, 0.0f, 1.0f, "%.10f");
        if (minAngularVelocity_.x > maxAngularVelocity_.x) {
            maxAngularVelocity_.x = minAngularVelocity_.x + 0.0000001f;
        }
        if (minAngularVelocity_.y > maxAngularVelocity_.y) {
            maxAngularVelocity_.y = minAngularVelocity_.y + 0.0000001f;
        }
        if (minAngularVelocity_.z > maxAngularVelocity_.z) {
            maxAngularVelocity_.z = minAngularVelocity_.z + 0.0000001f;
        }

        ImGui::DragFloat("scaleDecay_", &scaleDecay_, 0.0000001f, 0.0f, 1.0f, "%.10f");
        if (scaleDecay_ < 0.00000001f) {
            scaleDecay_ = 0.00000001f;
        }
        ImGui::DragInt("emitInterval_", &emitInterval_, 1, 0);
        ImGui::DragFloat("minScale_", &minScale_, 0.0000001f, 0.0f, 1.0f, "%.10f");
        ImGui::DragFloat("maxScale_", &maxScale_, 0.0000001f, minScale_, 1.0f, "%.10f");
        if (minScale_ > maxScale_) {
            maxScale_ = minScale_ + 0.0000001f;
        }
        ImGui::DragFloat3("color_", &color_.x, 0.01f);
        ImGui::DragFloat("toBatSpeed_", &toBatSpeed_, 0.0001f, 0.0f, 1.0f, "%.10f");
        ImGui::DragFloat("toBatScaleSpeed_", &toBatScaleSpeed_, 0.0001f, 0.0f, 1.0f, "%.10f");
        if (ImGui::Button("Save")) {
            JSON_OPEN("Resources/Data/GameScene/batteryParticles.json");
            JSON_OBJECT("batteryParticles");
            JSON_SAVE_BY_NAME("minSpeed_", minSpeed_);
            JSON_SAVE_BY_NAME("maxSpeed_", maxSpeed_);
            JSON_SAVE_BY_NAME("minAngularVelocity_", minAngularVelocity_);
            JSON_SAVE_BY_NAME("maxAngularVelocity_", maxAngularVelocity_);
            JSON_SAVE_BY_NAME("scaleDecay_", scaleDecay_);
            JSON_SAVE_BY_NAME("emitInterval_", emitInterval_);
            JSON_SAVE_BY_NAME("minScale_", minScale_);
            JSON_SAVE_BY_NAME("maxScale_", maxScale_);
            JSON_SAVE_BY_NAME("color_", color_);
            JSON_SAVE_BY_NAME("toBatSpeed_", toBatSpeed_);
            JSON_SAVE_BY_NAME("toBatScaleSpeed_", toBatScaleSpeed_);
            JSON_CLOSE();
        }
        ImGui::TreePop();

    }
    ImGui::End();
}

void BatteryParticles::Emit()
{
    auto newParticle = std::make_unique<Particle>();

    newParticle->type_ = ParticleType::ToCharge;

    newParticle->modelInstance_.SetModel(model_);
    newParticle->modelInstance_.SetUseLighting(false);

    newParticle->transform_.SetParent(&transform_, false);

    float startScale = rnd_.NextFloatRange(minScale_, maxScale_ + (radius_ * 0.2f));
    newParticle->transform_.scale = { startScale, startScale, startScale };

    Vector3 randomRotEuler = {
        rnd_.NextFloatRange(0.0f, 6.28f),
        rnd_.NextFloatRange(0.0f, 6.28f),
        rnd_.NextFloatRange(0.0f, 6.28f)
    };

    newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

    Vector3 spawnPos = { 0, 0, 0 };

    while (true) {
        Vector3 offset = {
            rnd_.NextFloatRange(-1.0f, 1.0f),
            rnd_.NextFloatRange(-1.0f, 1.0f),
            rnd_.NextFloatRange(-1.0f, 1.0f)
        };
        if (offset.LengthSquare() <= 1.0f) {
            spawnPos = offset * radius_;
            break;
        }
    }

    newParticle->transform_.translate = spawnPos;

    Vector3 randomDir = {
        rnd_.NextFloatRange(-1.0f, 1.0f),
        rnd_.NextFloatRange(-1.0f, 1.0f),
        rnd_.NextFloatRange(-1.0f, 1.0f)
    };
    if (randomDir.LengthSquare() != 0) {
        randomDir = randomDir.Normalized();
    }

    float speed = rnd_.NextFloatRange(minSpeed_, maxSpeed_);
    newParticle->velocity_ = randomDir * speed;

    newParticle->angularVelocity_ = {
        rnd_.NextFloatRange(minAngularVelocity_.x, maxAngularVelocity_.x),
        rnd_.NextFloatRange(minAngularVelocity_.y, maxAngularVelocity_.y),
        rnd_.NextFloatRange(minAngularVelocity_.z, maxAngularVelocity_.z)
    };

    newParticle->scaleSpeed_ = scaleDecay_;
    newParticle->modelInstance_.GetMaterials().emplace_back(material_);

    particles_.push_back(std::move(newParticle));
}


void BatteryParticles::ToBatEmit(const std::shared_ptr<Bats::Bat>& bat)
{
    auto newParticle = std::make_unique<Particle>();

    newParticle->type_ = ParticleType::ToBat;

    newParticle->modelInstance_.SetModel(model_);
    newParticle->modelInstance_.SetUseLighting(false);
    auto matInstance = std::make_shared<Material>();
    *matInstance = *toBatMaterial_;
    newParticle->modelInstance_.GetMaterials().emplace_back(matInstance);

    newParticle->transform_.SetParent(&transform_, false);

    newParticle->transform_.scale = { minScale_, minScale_, minScale_ };

    Vector3 randomRotEuler = {
        rnd_.NextFloatRange(0.0f, 6.28f),
        rnd_.NextFloatRange(0.0f, 6.28f),
        rnd_.NextFloatRange(0.0f, 6.28f)
    };
    newParticle->transform_.rotate = Quaternion::MakeFromEulerAngle(randomRotEuler);

    Vector3 spawnPos = { 0, 0, 0 };

    while (true) {
        Vector3 offset = {
            rnd_.NextFloatRange(-1.0f, 1.0f),
            rnd_.NextFloatRange(-1.0f, 1.0f),
            rnd_.NextFloatRange(-1.0f, 1.0f)
        };
        if (offset.LengthSquare() <= 1.0f) {
            spawnPos = offset * radius_;
            break;
        }
    }

    newParticle->transform_.translate = spawnPos;

    newParticle->transform_.UpdateMatrix();

    Vector3 direction = {
        rnd_.NextFloatRange(-0.5f,0.5f),
        rnd_.NextFloatRange(0.0f,1.0f),
        rnd_.NextFloatRange(0.0f, 1.0f)
    };

    newParticle->velocity_ = direction * toBatInitializeSpeed_;

    newParticle->targetBat_ = bat;

    newParticle->angularVelocity_ = {
        rnd_.NextFloatRange(minAngularVelocity_.x, maxAngularVelocity_.x),
        rnd_.NextFloatRange(minAngularVelocity_.y, maxAngularVelocity_.y),
        rnd_.NextFloatRange(minAngularVelocity_.z, maxAngularVelocity_.z)
    };

    newParticle->scaleSpeed_ = toBatScaleSpeed_;

    particles_.emplace_back(std::move(newParticle));
}
