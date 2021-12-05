#pragma once

#include <memory>

namespace geng
{
	struct void_tag { };

	template<typename T>
	struct FactoryTraits
	{
		using TArgument = void_tag;
	};

	template<typename T, typename A 
		= typename FactoryTraits<T>::TArgument>
	class IFactory
	{
	public:
		virtual ~IFactory() = default;

		// Result should be destroyable by "delete"
		virtual T* Create(A arg) = 0;
	};

	template<typename T, typename A>
	using FactorySharedPtr
		= std::shared_ptr<IFactory<T, A> >;

}