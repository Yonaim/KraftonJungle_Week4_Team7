#pragma once

#include "NewRenderer/Primitive/PrimitiveBase.h"

class ENGINE_API CPrimitiveCone : public CPrimitiveBase
{
public:
	static const FString Key;

	CPrimitiveCone();

	void Generate();
};
