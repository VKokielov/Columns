#pragma once

#include <memory>

namespace geng
{

	template<typename T>
	class IFactory
	{
	public:
		using TObj = T;

		virtual ~IFactory() = default;

		virtual T* Create() = 0;
	};

	template<typename T>
	using FactorySharedPtr
		= std::shared_ptr<IFactory<T> >;

}