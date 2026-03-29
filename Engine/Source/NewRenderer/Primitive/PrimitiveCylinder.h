#pragma once

#include "NewRenderer/Primitive/PrimitiveBase.h"

class ENGINE_API CPrimitiveCylinder : public CPrimitiveBase
{
public:
	static const FString Key;

	CPrimitiveCylinder();

	void Generate();
};
