#pragma once
#include "IFactory.h"
#include <type_traits>

namespace geng
{
	template<typename Product, 
				typename Base,
				typename Args = typename FactoryTraits<Base>::TArgument,
	            typename ProductArgs = Args>
	class StandardFactory : public IFactory<Base,Args>
	{
	private:
		static_assert(std::is_base_of_v<Base, Product>, "Product class P must be derived from B");

	public:
		P* Create(Args arg) override
		{
			if constexpr (std::is_same_v<decltype(arg), void_tag>)
			{
				return new P();
			}
			else
			{
				return new P(static_cast<ProductArgs>args>(arg));
			}
		}
	};


}