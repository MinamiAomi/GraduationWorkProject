#include "RenderManager.h"

#include "Core/ShaderManager.h"
#include "DefaultTextures.h"
#include "GameWindow.h"

#include "Framework/Engine.h"
#include "Editer/EditerManager.h"

#include "Input/Input.h"

static bool useGrayscale = true;

RenderManager* RenderManager::GetInstance() {
    static RenderManager instance;
    return &instance;
}

void RenderManager::Initialize() {
    graphics_ = Graphics::GetInstance();

    auto shaderManager = ShaderManager::GetInstance();
    shaderManager->Initialize();
    shaderManager->SetDirectory(std::filesystem::current_path() / SHADER_DIRECTORY);

    auto window = GameWindow::GetInstance();
    swapChain_.Create(window->GetHWND());

    DefaultTexture::Initialize();

    auto& swapChainBuffer = swapChain_.GetColorBuffer(0);
    finalImageBuffer_.Create(L"FinalImageBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), swapChainBuffer.GetFormat());

    skinningManager_.Initialize();
    geometryRenderingPass_.Initialize(swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight());
    lightingRenderingPass_.Initialize(swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight());
    skybox_.Initialize(lightingRenderingPass_.GetResult().GetRTVFormat(), geometryRenderingPass_.GetDepth().GetFormat());
    lineDrawer_.Initialize(lightingRenderingPass_.GetResult().GetRTVFormat());
    //particleCore_.Initialize(lightingRenderingPass_.GetResult().GetRTVFormat());
    spriteRenderer_.Initialize(finalImageBuffer_);
    //bloom_.Initialize(&lightingRenderingPass_.GetResult());
    postEffect_.Initialize(finalImageBuffer_);

    //    modelRenderer.Initialize(mainColorBuffer_, mainDepthBuffer_);
    transition_.Initialize();

    fxaa_.Initialize(&lightingRenderingPass_.GetResult());

    timer_.Initialize();

    frameCount_ = 0;
}

void RenderManager::Finalize() {
    DefaultTexture::Finalize();
}

void RenderManager::Render() {

    uint32_t targetSwapChainBufferIndex = (swapChain_.GetCurrentBackBufferIndex() + 1) % SwapChain::kNumBuffers;

    auto camera = camera_.lock();
    auto sunLight = sunLight_.lock();

    commandContext_.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);

    const float deltaSecond = 1 / 60.0f;
    const float daySpeed = 1;
    sky_.Update(deltaSecond / daySpeed);

    skinningManager_.Update(commandContext_);

    //particleCore_.Dispatch(commandContext_);

    if (camera && sunLight) {
        // 影、スペキュラ
        modelSorter_.Sort(*camera);

        geometryRenderingPass_.Render(commandContext_, *camera, modelSorter_);

        lightingRenderingPass_.Render(commandContext_, geometryRenderingPass_, *camera, *sunLight);

        commandContext_.TransitionResource(lightingRenderingPass_.GetResult(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandContext_.TransitionResource(geometryRenderingPass_.GetDepth(), D3D12_RESOURCE_STATE_DEPTH_READ);
        commandContext_.SetViewportAndScissorRect(0, 0, lightingRenderingPass_.GetResult().GetWidth(), lightingRenderingPass_.GetResult().GetHeight());
        commandContext_.SetRenderTarget(lightingRenderingPass_.GetResult().GetRTV(), geometryRenderingPass_.GetDepth().GetDSV());
        skybox_.SetWorldMatrix(Matrix4x4::MakeAffineTransform({ 1.0f, 1.0f, 1.0f }, Quaternion::identity, camera->GetPosition()));
        //skybox_.Render(commandContext_, *camera);

        commandContext_.SetRenderTarget(lightingRenderingPass_.GetResult().GetRTV());
        commandContext_.SetViewportAndScissorRect(0, 0, lightingRenderingPass_.GetResult().GetWidth(), lightingRenderingPass_.GetResult().GetHeight());
        lineDrawer_.Render(commandContext_, *camera);

        //particleCore_.Render(commandContext_, *camera);
    }

    //bloom_.Render(commandContext_);
    fxaa_.Render(commandContext_);

    commandContext_.TransitionResource(finalImageBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext_.SetRenderTarget(finalImageBuffer_.GetRTV());
    commandContext_.SetViewportAndScissorRect(0, 0, finalImageBuffer_.GetWidth(), finalImageBuffer_.GetHeight());

    
    postEffect_.Render(commandContext_, fxaa_.GetResult());
    spriteRenderer_.Render(commandContext_, 0.0f, 0.0f, (float)finalImageBuffer_.GetWidth(), (float)finalImageBuffer_.GetHeight());

    auto& swapChainBuffer = swapChain_.GetColorBuffer(targetSwapChainBufferIndex);

    commandContext_.CopyBuffer(swapChainBuffer, finalImageBuffer_);

#ifdef ENABLE_IMGUI
    // スワップチェーンに描画
    commandContext_.TransitionResource(swapChainBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandContext_.FlushResourceBarriers();
    commandContext_.SetRenderTarget(swapChainBuffer.GetRTV(ColorBuffer::RTV::SRGB));
    //commandContext_.ClearColor(swapChainBuffer);
    commandContext_.SetViewportAndScissorRect(0, 0, swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight());

    Engine::GetEditerManager()->RenderToColorBuffer(commandContext_);
#endif // ENABLE_IMGUI

    commandContext_.TransitionResource(swapChainBuffer, D3D12_RESOURCE_STATE_PRESENT);

    // コマンドリスト完成(クローズ)
    commandContext_.Close();
    commandContext_.Finish(false);

    // バックバッファをフリップ
    swapChain_.Present();
    frameCount_++;
    // シグナルを発行し待つ
    auto& commandManager = graphics_->GetCommandManager();
    commandManager.GetCommandQueue().WaitForIdle();

    commandManager.Execute();
    graphics_->GetReleasedObjectTracker().FrameIncrementForRelease();

    timer_.KeepFrameRate(60);

}