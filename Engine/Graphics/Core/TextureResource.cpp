#include "TextureResource.h"

#include "Externals/DirectXTex/Include/DirectXTex.h"

#include "Helper.h"
#include "Graphics.h"
#include "CommandContext.h"
#include "UploadBuffer.h"

void TextureResource::Create(const std::filesystem::path& path, bool useSRGB) {
    // 中間リソースをコピーする
    CommandContext commandContext;
    commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
    Create(commandContext, path, useSRGB);
    commandContext.Finish(true);
}

void TextureResource::Create(size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, void* dataBegin) {
    // 中間リソースをコピーする
    CommandContext commandContext;
    commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
    Create(commandContext, rowPitchBytes, width, height, format, dataBegin);
    commandContext.Finish(true);
}


void TextureResource::Create(CommandContext& commandContext, const std::filesystem::path& path, bool useSRGB) {
    auto device = Graphics::GetInstance()->GetDevice();

    // ファイルを読み込む
    DirectX::ScratchImage image{};
    if (path.extension() == ".dds") {
        ASSERT_IF_FAILED(DirectX::LoadFromDDSFile(path.wstring().c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image));
    }
    else {
        ASSERT_IF_FAILED(DirectX::LoadFromWICFile(path.wstring().c_str(), useSRGB ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_FORCE_RGB, nullptr, image));
    }
    // ミップマップを生成
    DirectX::ScratchImage mipImages{};
    if (DirectX::IsCompressed(image.GetMetadata().format)) {
        mipImages = std::move(image);
    }
    else {
        ASSERT_IF_FAILED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages));
    }

    // リソースを生成
    auto& metadata = mipImages.GetMetadata();
    desc_.Width = UINT(metadata.width);
    desc_.Height = UINT(metadata.height);
    desc_.MipLevels = UINT16(metadata.mipLevels);
    desc_.DepthOrArraySize = UINT16(metadata.arraySize);
    desc_.Format = metadata.format;
    desc_.SampleDesc.Count = 1;
    desc_.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

    // 中間リソースを読み込む
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

    CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
    CreateResource(L"TextureResource", heapProp, desc_, D3D12_RESOURCE_STATE_COPY_DEST);

    UploadResource(commandContext, UINT(subresources.size()), subresources.data());

    // ビューを生成
    CreateView(metadata.IsCubemap());
}

void TextureResource::Create(CommandContext& commandContext, size_t rowPitchBytes, size_t width, size_t height, DXGI_FORMAT format, void* dataBegin, bool isCubeMap) {

    CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
    desc_ = CD3DX12_RESOURCE_DESC::Tex2D(format, width, UINT(height), isCubeMap ? 6 : 1, 1);
    CreateResource(L"TextureResource", heapProp, desc_, D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pData = dataBegin;
    subresourceData.RowPitch = rowPitchBytes;
    subresourceData.SlicePitch = rowPitchBytes * height;

    UploadResource(commandContext, 1, &subresourceData);

    CreateView(isCubeMap);
}

void TextureResource::Create(CommandContext& commandContext, PixelBuffer& pixelBuffer) {
    // テクスチャ配列は禁止
    assert(pixelBuffer.GetArraySize() == 1);
    CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_DEFAULT);
    desc_ = CD3DX12_RESOURCE_DESC::Tex2D(pixelBuffer.GetFormat(), pixelBuffer.GetWidth(), pixelBuffer.GetHeight(), 1, 1);
    CreateResource(L"TextureResource", heapProp, desc_, D3D12_RESOURCE_STATE_COPY_DEST);

    commandContext.TransitionResource(pixelBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandContext.CopyBuffer(*this, pixelBuffer);
    commandContext.TransitionResource(*this, D3D12_RESOURCE_STATE_GENERIC_READ);
    
    CreateView(false);
}

void TextureResource::CreateView(bool isCubeMap) {
    auto graphics = Graphics::GetInstance();
    auto device = graphics->GetDevice();

    if (srvHandle_.IsNull()) {
        srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc_.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    if (isCubeMap) {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.MipLevels = UINT_MAX;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    }
    else {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc_.MipLevels;
    }
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, srvHandle_);
}

void TextureResource::UploadResource(CommandContext& commandContext, size_t numSubresources, const D3D12_SUBRESOURCE_DATA* subresources) {
    uint64_t intermediateSize = GetRequiredIntermediateSize(resource_.Get(), 0, UINT(numSubresources));

    UploadBuffer intermediateResource;
    intermediateResource.Create(L"TextureResource IntermediateResource", intermediateSize);

    UpdateSubresources(commandContext, resource_.Get(), intermediateResource, 0, 0, UINT(numSubresources), subresources);
    commandContext.TransitionResource(*this, D3D12_RESOURCE_STATE_GENERIC_READ);
}

