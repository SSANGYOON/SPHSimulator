#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
struct Particle
{
    Vector3 position;
    float density;
    Vector3 velocity;
    float pressure;
};