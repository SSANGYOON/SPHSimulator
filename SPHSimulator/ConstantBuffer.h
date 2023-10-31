#pragma once

#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name

#define CBSLOT_TRANSFORM 0
#define CBSLOT_MATERIAL 1
#define CBSLOT_PARTICLE 2
#define CBSLOT_PARTICLESORT 3
#define CBSLOT_PARTICLERENDER 4

#include "Enums.h"

enum class Constantbuffer_Type : UINT8
{
	TRANSFORM,
	MATERIAL,
	PARTICLE,
	PARTICLESORT,
	PARTICLERENDER,
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
	float massPoly6Product;
	float selfDens;
	float gasConstant;
	float restDensity;
	float mass;
	float spikyGrad;
	float spikyLap;
	float viscosity;
	float gravity;
	float deltaTime;
};

CBUFFER(ParticleSortCB, CBSLOT_PARTICLESORT)
{
	UINT j;
	UINT k;
	Vector2 padding;
};

CBUFFER(ParticleRenderCB, CBSLOT_PARTICLERENDER)
{
	float blurScale;
	float blurDepthFalloff;
	int filterRadius;
	float filterPadding;
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