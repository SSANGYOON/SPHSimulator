#include "pch.h"
#include "Graphics.h"

unique_ptr<Graphics> GEngine = make_unique<Graphics>();

UINT NextPowerOf2(UINT number)
{
	UINT r = 1;
	while (r < number)
	{
		r <<= 1;
	}

	return r;
}