#pragma once

#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name

#define CBSLOT_TRANSFORM 0
#define CBSLOT_MATERIAL 1
#define CBSLOT_PARTICLE 2
#define CBSLOT_PARTICLESORT 3
#define CBSLOT_PARTICLERENDER 4
#define CBSLOT_SDFPROPERTY 5
#define CBSLOT_OBSTACLEBUFFER 6

#include "Enums.h"

enum class Constantbuffer_Type : UINT8
{
	TRANSFORM,
	MATERIAL,
	PARTICLE,
	PARTICLESORT,
	PARTICLERENDER,
	SDFPROPERTY,
	OBSTACLE,
	END
};

CBUFFER(TransformCB, CBSLOT_TRANSFORM)
{
	Matrix world;
	Matrix view;
	Matrix viewInv;
	Matrix projection;
	Matrix projectionInv;
};

CBUFFER(MaterialCB, CBSLOT_MATERIAL)
{
	int     tex0_On;
	int     tex1_On;
	int     tex2_On;
	int     tex3_On;

	Vector2 viewPort;
	float nearClip;
	float farClip;
};

CBUFFER(ParticleCB, CBSLOT_PARTICLE)
{
	UINT particlesNum;
	float radius;
	float restDensity;
	float mass;
	Vector3 boundaryCentor;
	float viscosity;
	Vector3 boundarySize;
	float gravity;
	float deltaTime;
	UINT tableSize;
	Vector2 settingsPadding;
};

CBUFFER(ParticleSortCB, CBSLOT_PARTICLESORT)
{
	UINT j;
	UINT k;
	Vector2 padding;
};

CBUFFER(ParticleRenderCB, CBSLOT_PARTICLERENDER)
{
	float blurDepthFalloff;
	int filterRadius;
	float absorbanceCoff;
	float renderPadding;
	Vector3 SpecularColor;
	float SpecularPower;
	float SpecularIntensity;
	Vector3 fluidColor;
};
CBUFFER(SDFPropertyCB, CBSLOT_SDFPROPERTY)
{
	tuple<UINT, UINT, UINT> size;
	float cellSize;
};

CBUFFER(ObstacleCB, CBSLOT_OBSTACLEBUFFER)
{
	Vector3 origin;
	float padding1;
	Vector3 offset;
	float padding2;
	tuple<UINT, UINT, UINT> size;
	float padding3;
};


struct alignas(16) LightInfo
{
	Vector3 dir;
	float SpotlightFov;
	Vector3 position;
	float range;
	Vector3 color;
	UINT type;
};

class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	void Init(Constantbuffer_Type type, UINT size);
	void SetData(void* Data);
	void SetPipline(ShaderStage stage);

private:
	Constantbuffer_Type _type;
	ComPtr<ID3D11Buffer> _buffer;
	D3D11_BUFFER_DESC desc;
};