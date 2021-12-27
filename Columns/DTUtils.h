#pragma once
#include "IDataTree.h"
#include "DTConstruct.h"

namespace geng::data
{

	bool SerializeDataTree(const std::shared_ptr<IDatum>& pRoot,
		ITreeSerializer& serializer);

}