#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;
struct Particle
{
    Vector3 position, velocity, acceleration, force;
    float density;
    float pressure;
    uint16_t hash;
};