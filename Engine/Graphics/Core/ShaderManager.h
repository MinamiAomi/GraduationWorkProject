#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <wrl/client.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

enum class ShaderType {
    Vertex, 
    Hull,
    Domain,
    Geometry,
    Pixel,
    Compute,
    Library
};

class ShaderCompileOptions {
    friend class ShaderManager;
public:
    ShaderCompileOptions();

    ShaderCompileOptions& SetEntryPoint(const std::wstring& entryPoint);
    ShaderCompileOptions& SetProfile(ShaderType shaderType, int majorVersion, int minorVersion);
    ShaderCompileOptions& AddDefine(const std::wstring& define);
    ShaderCompileOptions& EnableDebug();
    ShaderCompileOptions& EnableOptimizations();
    ShaderCompileOptions& EnableRowMajor();

    void Clear();

private:
    std::vector<LPCWSTR> arguments_;
    std::vector<std::wstring> buffer_;

};

class ShaderManager {
public:
    static ShaderManager* GetInstance();

    enum Type {
        kVertex,
        kPixel,
        kGeometry,
        kHull,
        kDomain,
        kCompute,

        kNumTypes
    };

    void Initialize();
    Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::filesystem::path& path, ShaderCompileOptions& options);
    Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::filesystem::path& path, Type type);
    Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::filesystem::path& path, ShaderType type, int majorVersion, int minorVersion);

    void SetDirectory(const std::filesystem::path& directory) { directory_ = directory; }

private:
    ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& path, LPCWSTR* arguments, UINT32 numArguments);

    Microsoft::WRL::ComPtr<IDxcUtils> utils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

    std::vector<std::wstring> loadedShaderList_;
    std::filesystem::path directory_;
};