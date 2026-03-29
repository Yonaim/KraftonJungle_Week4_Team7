#pragma once

#include "NewRenderer/Primitive/PrimitiveBase.h"

class ENGINE_API CPrimitiveQuad : public CPrimitiveBase
{
public:
	static const FString Key;

	CPrimitiveQuad();

	void Generate();
};
