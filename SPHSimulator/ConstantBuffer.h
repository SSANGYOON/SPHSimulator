#pragma once

#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name

#define CBSLOT_TRANSFORM 0
#define CBSLOT_MATERIAL 1
#define CBSLOT_VISUALEFFECT 2
#define CBSLOT_LIGHT 3

#include "Enums.h"

enum class Constantbuffer_Type : UINT8
{
	TRANSFORM,
	MATERIAL,
	VISUALEFFECT,
	LIGHT,
	PARTICLE,
	BLOOM,
	END
};

CBUFFER(TransformCB, CBSLOT_TRANSFORM)
{
	Matrix world;
	Matrix view;
	Matrix projection;
	int entity;
	Vector3 padding;
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

CBUFFER(VECB, CBSLOT_VISUALEFFECT)
{
	float time;
	float DeltaTime;
	Vector2 ViewPort;
	Vector4 ScissorRect;
	Vector4 FadeColor;
};

CBUFFER(LightCB, CBSLOT_LIGHT)
{
	UINT		lightCount;
	Vector3		padding;
	LightInfo	lights[50];
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