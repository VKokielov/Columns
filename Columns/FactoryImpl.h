#pragma once
#include "IFactory.h"
#include <type_traits>
#include <tuple>

namespace geng
{

	namespace factory_helpers
	{
		// This little class allows us to use make_from_tuple in the factory
		// implementation
		template<typename T>
		class RawPtrHolder
		{
		public:
			template<typename ... Args>
			RawPtrHolder(Args&& ... args)
				:m_ptr(new T(std::forward<Args>(args)...))
			{ }

			T* GetPtr() const { return m_ptr; }
		private:
			T*  m_ptr;
		};
	}


	// A forwarding factory stores a list of arguments which it then forwards to the 
	// constructor.  It represents a mechanism for delaying and multiplying construction
	// It behaves much like Lisp's procedure + environment pairs, but of course the usual
	// C++ scope/lifetime complications apply.  Avoid using reference types in Args unless you know
	// EXACTLY what you're doing!
	template<typename Product, 
				typename Base,
				typename ... Args>
	class ForwardingFactory : public IFactory<Base>
	{
	private:
		static_assert(std::is_base_of_v<Base, Product>, "Product class P must be derived from B");

	public:
		
		ForwardingFactory(Args&& ... args)
			:m_args(std::forward<Args>(args)...)
		{ }


		Product* Create() override
		{
			auto productHolder
				= std::make_from_tuple<factory_helpers::RawPtrHolder<Product> >
					(m_args);

			return productHolder.GetPtr();
		}

	private:
		std::tuple<Args...>  m_args;
	};


	// Deliberately with value arguments rather than genrefs
	template<typename Product, typename Base, typename ... Args>
	IFactory<Base>* CreateFactoryWithArgs(Args...args)
	{
		return new ForwardingFactory<Product, Base, Args...>(std::forward<Args>(args)...);
	}


}