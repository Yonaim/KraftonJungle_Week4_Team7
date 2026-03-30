#pragma once

#include "Renderer/Primitive/PrimitiveBase.h"

class ENGINE_API CPrimitiveRing : public CPrimitiveBase
{
public:
	static const FString Key;

	CPrimitiveRing();

	void Generate();
};
