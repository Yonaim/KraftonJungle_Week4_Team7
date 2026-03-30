#pragma once

#include "Renderer/Primitive/PrimitiveBase.h"

class ENGINE_API CPrimitiveTriangle : public CPrimitiveBase
{
public:
	static const FString Key;

	CPrimitiveTriangle();

	void Generate();
};
