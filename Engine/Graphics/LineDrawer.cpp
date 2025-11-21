#include "LineDrawer.h"

#include <array>

#include "Core/Helper.h"
#include "Core/ShaderManager.h"
#include "Core/ColorBuffer.h"
#include "Core/DepthBuffer.h"
#include "Core/CommandContext.h"

namespace {
	const wchar_t kVertexShader[] = L"LineVS.hlsl";
	const wchar_t kPixelShader[] = L"LinePS.hlsl";
}

void LineDrawer::Initialize(DXGI_FORMAT rtvFormat) {
	CD3DX12_ROOT_PARAMETER rootParameters[1]{};
	rootParameters[0].InitAsConstantBufferView(0);

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = _countof(rootParameters);
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignature_.Create(L"LineDrawer RootSignature", rootSignatureDesc);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

	pipelineStateDesc.pRootSignature = rootSignature_;

	D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		 { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_INPUT_LAYOUT_DESC inputLayout{};
	inputLayout.NumElements = _countof(inputElements);
	inputLayout.pInputElementDescs = inputElements;
	pipelineStateDesc.InputLayout = inputLayout;

	auto shaderManager = ShaderManager::GetInstance();
	auto vs = shaderManager->Compile(kVertexShader, ShaderManager::kVertex);
	auto ps = shaderManager->Compile(kPixelShader, ShaderManager::kPixel);

	pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
	pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

	pipelineStateDesc.BlendState = Helper::BlendDisable;
	pipelineStateDesc.DepthStencilState = Helper::DepthStateDisabled;
	pipelineStateDesc.RasterizerState = Helper::RasterizerNoCull;
	// 前面カリング
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = rtvFormat;
	pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	pipelineStateDesc.SampleDesc.Count = 1;
	pipelineState_.Create(L"LineDrawer PipelineState", pipelineStateDesc);

}

void LineDrawer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
	vertices_.emplace_back(start, color);
	vertices_.emplace_back(end, color);
}

void LineDrawer::ObbDraw(const Vector3& center, const Vector3& size,  const Quaternion& rotation, const Vector4& color) {

	Vector3 halfSize = size * 0.5f;

	Vector3 localCorners[8] = {
		{ -halfSize.x, -halfSize.y, -halfSize.z },
		{ +halfSize.x, -halfSize.y, -halfSize.z },
		{ -halfSize.x, +halfSize.y, -halfSize.z },
		{ +halfSize.x, +halfSize.y, -halfSize.z },
		{ -halfSize.x, -halfSize.y, +halfSize.z },
		{ +halfSize.x, -halfSize.y, +halfSize.z },
		{ -halfSize.x, +halfSize.y, +halfSize.z },
		{ +halfSize.x, +halfSize.y, +halfSize.z }
	};

	std::array<Vector3, 8> worldCorners;
	for (int i = 0; i < 8; ++i) {
		Vector3 rotatedCorner = rotation * localCorners[i];

		worldCorners[i] = center + rotatedCorner;
	}

	AddLine(worldCorners[0], worldCorners[1], color);
	AddLine(worldCorners[1], worldCorners[3], color);
	AddLine(worldCorners[3], worldCorners[2], color);
	AddLine(worldCorners[2], worldCorners[0], color);

	AddLine(worldCorners[4], worldCorners[5], color);
	AddLine(worldCorners[5], worldCorners[7], color);
	AddLine(worldCorners[7], worldCorners[6], color);
	AddLine(worldCorners[6], worldCorners[4], color);

	AddLine(worldCorners[0], worldCorners[4], color);
	AddLine(worldCorners[1], worldCorners[5], color);
	AddLine(worldCorners[2], worldCorners[6], color);
	AddLine(worldCorners[3], worldCorners[7], color);
}

void LineDrawer::DrawBox(const Vector3& center, const Vector3& size, const Vector4& color)
{
	Vector3 halfSize = size * 0.5f;
	Vector3 min = center - halfSize;
	Vector3 max = center + halfSize;

	// 8頂点
	Vector3 p0 = { min.x, min.y, min.z };
	Vector3 p1 = { max.x, min.y, min.z };
	Vector3 p2 = { min.x, max.y, min.z };
	Vector3 p3 = { max.x, max.y, min.z };
	Vector3 p4 = { min.x, min.y, max.z };
	Vector3 p5 = { max.x, min.y, max.z };
	Vector3 p6 = { min.x, max.y, max.z };
	Vector3 p7 = { max.x, max.y, max.z };

	// 前面 (Z-)
	AddLine(p0, p1, color);
	AddLine(p1, p3, color);
	AddLine(p3, p2, color);
	AddLine(p2, p0, color);

	// 後面 (Z+)
	AddLine(p4, p5, color);
	AddLine(p5, p7, color);
	AddLine(p7, p6, color);
	AddLine(p6, p4, color);

	// 側面接続
	AddLine(p0, p4, color);
	AddLine(p1, p5, color);
	AddLine(p2, p6, color);
	AddLine(p3, p7, color);
}

void LineDrawer::DrawSphere(const Vector3& center, float radius, const Vector4& color)
{
	const int kSegments = 16; // 分割数
	const float kAngleStep = (2.0f * Math::Pi) / kSegments;

	for (int i = 0; i < kSegments; ++i) {
		float theta = i * kAngleStep;
		float nextTheta = (i + 1) * kAngleStep;

		float sin0 = std::sin(theta) * radius;
		float cos0 = std::cos(theta) * radius;
		float sin1 = std::sin(nextTheta) * radius;
		float cos1 = std::cos(nextTheta) * radius;

		// XY平面の円 
		AddLine(
			center + Vector3(cos0, sin0, 0.0f),
			center + Vector3(cos1, sin1, 0.0f),
			color
		);

		// XZ平面の円
		AddLine(
			center + Vector3(cos0, 0.0f, sin0),
			center + Vector3(cos1, 0.0f, sin1),
			color
		);

		// YZ平面の円
		AddLine(
			center + Vector3(0.0f, cos0, sin0),
			center + Vector3(0.0f, cos1, sin1),
			color
		);
	}
}

void LineDrawer::DrawCone(const Vector3& center, float radius, float height, const Quaternion& rotation, const Vector4& color)
{
	const int kSegments = 16;
	const float kAngleStep = (2.0f * Math::Pi) / kSegments;
	
	// 先端の位置 (Y軸プラス方向を高さとする)
	// 
	Vector3 topLocal = { 0.0f, height, 0.0f };
	Vector3 topWorld = center + (rotation * topLocal);

	// 中心軸
	AddLine(center, topWorld, color);

	for (int i = 0; i < kSegments; ++i) {
		float theta = i * kAngleStep;
		float nextTheta = (i + 1) * kAngleStep;

		// 底面の円周上の点 (XZ平面)
		Vector3 p0Local = { std::cos(theta) * radius, 0.0f, std::sin(theta) * radius };
		Vector3 p1Local = { std::cos(nextTheta) * radius, 0.0f, std::sin(nextTheta) * radius };

		// 回転を適用してワールド座標へ
		Vector3 p0World = center + (rotation * p0Local);
		Vector3 p1World = center + (rotation * p1Local);

		// 底面の円を描画
		AddLine(p0World, p1World, color);

		// 底面から先端への線 (4分割に1回引くなど間引くと見やすいですが、ここでは全て引きます)
		if (i % 4 == 0) {
			AddLine(p0World, topWorld, color);
		}
	}
}

void LineDrawer::Render(CommandContext& commandContext, const Camera& camera) {
	assert(vertices_.size() % 2 == 0);
	if (vertices_.empty()) { return; }
	commandContext.SetRootSignature(rootSignature_);
	commandContext.SetPipelineState(pipelineState_);
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	Matrix4x4 viewProjectionMatrix = camera.GetViewProjectionMatrix();
	commandContext.SetDynamicConstantBufferView(0, sizeof(viewProjectionMatrix), &viewProjectionMatrix);
	commandContext.SetDynamicVertexBuffer(0, vertices_.size(), sizeof(vertices_[0]), vertices_.data());
	commandContext.Draw((UINT)vertices_.size());
	vertices_.clear();
}
