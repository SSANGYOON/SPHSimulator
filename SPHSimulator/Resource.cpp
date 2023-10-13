#include "pch.h"
#include "Resource.h"

Resource::Resource(RESOURCE_TYPE type)
	:_type(type)
{
	static UINT idGenerator = 1;
	_id = idGenerator;
	idGenerator++;
}

Resource::~Resource()
{
}
