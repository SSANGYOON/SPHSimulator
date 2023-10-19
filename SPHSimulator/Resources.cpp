#include "pch.h"
#include "Resources.h"
#include "Mesh.h"
#include "Shader.h"
#include "ComputeShader.h"

void Resources::CreateDefaultResource()
{
#pragma region POINT MESH
	Vertex v = {};
	std::shared_ptr<Mesh> pointMesh = std::make_shared<Mesh>();
	Resources::Insert<Mesh>(L"PointMesh", pointMesh);
	pointMesh->CreateVertexBuffer(&v, 1);
	UINT pointIndex = 0;
	pointMesh->CreateIndexBuffer(&pointIndex, 1);
#pragma endregion

#pragma region Default Rect
	shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	Resources::Insert<Mesh>(L"RectMesh", mesh);
	Vertex vertexes[4] = {};

	vertexes[0].pos = Vector3(-0.5f, 0.5f, 0.f);
	vertexes[0].Color = Vector4(1.f, 0.0f, 0.f, 1.0f);
	vertexes[0].uv = Vector2(0.f, 0.f);

	vertexes[1].pos = Vector3(0.5f, 0.5f, 0.f);
	vertexes[1].Color = Vector4(0.f, 1.f, 0.f, 1.0f);
	vertexes[1].uv = Vector2(1.0f, 0.0f);

	vertexes[2].pos = Vector3(0.5f, -0.5f, 0.f);
	vertexes[2].Color = Vector4(0.f, 0.f, 1.f, 1.0f);
	vertexes[2].uv = Vector2(1.0f, 1.0f);

	vertexes[3].pos = Vector3(-0.5f, -0.5f, 0.f);
	vertexes[3].Color = Vector4(1.f, 0.f, 0.f, 1.0f);
	vertexes[3].uv = Vector2(0.0f, 1.0f);

	mesh->CreateVertexBuffer(vertexes, 4);

	std::vector<UINT> indexes;
	indexes.push_back(0);
	indexes.push_back(1);
	indexes.push_back(2);

	indexes.push_back(2);
	indexes.push_back(3);
	indexes.push_back(0);

	mesh->CreateIndexBuffer(indexes.data(), (UINT)indexes.size());
#pragma endregion

#pragma region OutlinedRect
	shared_ptr<Mesh> outlinedRect = std::make_shared<Mesh>();
	Resources::Insert<Mesh>(L"OutlinedRect", outlinedRect);
	outlinedRect->CreateVertexBuffer(vertexes, 4);

	outlinedRect->CreateIndexBuffer(indexes.data(), (UINT)indexes.size());
#pragma endregion

#pragma region DebugCircle
	{
		vector<Vertex> circleVertexes;
		Vertex center = {};
		center.pos = Vector3(0.0f, 0.0f, 0.f);
		center.uv = Vector2::Zero;

		circleVertexes.push_back(center);

		int iSlice = 80;
		float fRadius = 0.5f;
		float fTheta = XM_2PI / (float)iSlice;


		for (size_t i = 0; i < iSlice; i++)
		{
			Vertex vtx = {};
			vtx.pos = Vector3
			(
				fRadius * cosf(fTheta * (float)i)
				, fRadius * sinf(fTheta * (float)i)
				, -0.00001f);

			circleVertexes.push_back(vtx);
		}

		vector<UINT> circleIndexes;
		for (size_t i = 0; i < iSlice; i++)
		{
			circleIndexes.push_back(i + 1);
		}
		circleIndexes.push_back(1);

		std::shared_ptr<Mesh> cirlceMesh = std::make_shared<Mesh>();
		Resources::Insert<Mesh>(L"CircleMesh", cirlceMesh);
		cirlceMesh->CreateVertexBuffer(circleVertexes.data(), circleVertexes.size());
		cirlceMesh->CreateIndexBuffer(circleIndexes.data(), circleIndexes.size());
	}
#pragma endregion

#pragma region 2DCircle
	{
		vector<Vertex> circleVertexes;
		Vertex center = {};
		center.pos = Vector3(0.0f, 0.0f, 0.1f);
		center.uv = Vector2::Zero;

		circleVertexes.push_back(center);

		int iSlice = 120;
		float fRadius = 1.f;
		float fTheta = XM_2PI / (float)iSlice;


		for (size_t i = 0; i < iSlice; i++)
		{
			Vertex vtx = {};
			vtx.pos = Vector3
			(
				fRadius * cosf(fTheta * (float)i) / 2.f
				, fRadius * sinf(fTheta * (float)i) / 2.f
				, 0.1f
			);

			circleVertexes.push_back(vtx);
		}

		vector<UINT> circleIndexes;
		for (size_t i = 1; i <= iSlice; i++)
		{
			circleIndexes.push_back(i);
			circleIndexes.push_back(i % iSlice + 1);
			circleIndexes.push_back(0);
		}

		std::shared_ptr<Mesh> cirlceMesh = std::make_shared<Mesh>();
		Resources::Insert<Mesh>(L"Circle2D", cirlceMesh);
		cirlceMesh->CreateVertexBuffer(circleVertexes.data(), (UINT)circleVertexes.size());
		cirlceMesh->CreateIndexBuffer(circleIndexes.data(), circleIndexes.size());
	}
#pragma endregion

#pragma region Lcosahedron
	shared_ptr<Mesh> Lcosahedron = std::make_shared<Mesh>();
	Resources::Insert<Mesh>(L"Lcosahedron", Lcosahedron);
	wstring path = std::filesystem::current_path().parent_path().wstring() + L"/SHADER_SOURCE/models/lowsphere.obj";
	Lcosahedron->Load(path);
#pragma endregion

#pragma region HardCoded3DShader
	{
		ShaderInfo _info;
		ShaderEntry _entry;

		shared_ptr<Shader> HardCoded3DShader = std::make_shared<Shader>();
		Resources::Insert<Shader>(L"HardCoded3DShader", HardCoded3DShader);
		_info.bst = BSType::AlphaBlend;
		_info.dst = DSType::Less;
		_info.rst = RSType::SolidNone;
		_info.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		_entry = {};
		_entry.VS = true;
		_entry.PS = true;
		HardCoded3DShader->CreateShader(_info, _entry, L"HardCoded3DShader.hlsl", false);
	}
#pragma endregion

#pragma region TestShader
	ShaderInfo _info;
	ShaderEntry _entry;

	shared_ptr<Shader> TestShader = std::make_shared<Shader>();
	Resources::Insert<Shader>(L"TestShader", TestShader);
	_info.bst = BSType::AlphaBlend;
	_info.dst = DSType::Less;
	_info.rst = RSType::SolidNone;
	_info.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	_entry = {};
	_entry.VS = true;
	_entry.PS = true;
	TestShader->CreateShader(_info, _entry, L"TestShader.hlsl");
#pragma endregion

#pragma region CountingShader
	shared_ptr<ComputeShader> CountingShader = std::make_shared<ComputeShader>();
	Resources::Insert<ComputeShader>(L"CountingShader", CountingShader);
	CountingShader->Create(L"CreateCountingBuffer.hlsl");
#pragma endregion

#pragma region PrefixSumOnThreadGroup
	shared_ptr<ComputeShader> PrefixSumOnThreadGroupShader = std::make_shared<ComputeShader>();
	Resources::Insert<ComputeShader>(L"PrefixSumOnThreadGroupShader", PrefixSumOnThreadGroupShader);
	PrefixSumOnThreadGroupShader->Create(L"PrefixSumOnThreadGroup.hlsl");
#pragma endregion

#pragma region PrefixSumOnGroupSum
	shared_ptr<ComputeShader> PrefixSumOnGroupSum = std::make_shared<ComputeShader>();
	Resources::Insert<ComputeShader>(L"PrefixSumOnGroupSumShader", PrefixSumOnGroupSum);
	PrefixSumOnGroupSum->Create(L"PrefixSumOnGroupSum.hlsl");
#pragma endregion

#pragma region PrefixSumComplete
	shared_ptr<ComputeShader> PrefixSumComplete = std::make_shared<ComputeShader>();
	Resources::Insert<ComputeShader>(L"PrefixSumCompleteShader", PrefixSumComplete);
	PrefixSumComplete->Create(L"CompletePrefixSum.hlsl");
#pragma endregion

#pragma region CountingSortComplete
	shared_ptr<ComputeShader> CountingSortCompletShader = std::make_shared<ComputeShader>();
	Resources::Insert<ComputeShader>(L"CompleteCountingSort", CountingSortCompletShader);
	CountingSortCompletShader->Create(L"CompleteCountingSort.hlsl");
#pragma endregion
}
