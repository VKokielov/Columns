#pragma once

#include <variant>
#include <functional>

namespace geng
{

	template<typename Derived, typename InitState, typename ... States>
	class SimStateDispatcher
	{
	public:

		SimStateDispatcher()
		{
			m_varStates.emplace<InitState>();
		}
		
		size_t GetStateIndex() const
		{
			return m_varStates.index();
		}

		template<typename Callable, typename ... Args>
		void DispatchInvoke(Callable&& callable, Args&&...args)
		{
			auto visitor = [&](auto& state)
			{
				std::invoke(callable, state, std::forward<Args>(args)...);
			};

			std::visit(visitor, m_varStates);
		}

		template<typename Obj, typename ... Args>
		void Dispatch(Obj& obj, Args&&...args)
		{
			auto visitor = [&](auto& state)
			{
				obj.OnState(state, std::forward<Args>(args)...);
			};

			std::visit(visitor, m_varStates);
		}

		template<typename TargetState, typename Obj, typename ... Args>
		void Transition(Obj& obj, Args&...args)
		{
			// Nop when the states are identical
			if (std::holds_alternative<TargetState>(m_varStates))
			{
				return;
			}

			auto exitVisitor = [&](auto& state)
			{
				obj.OnExitState(state,std::forward<Args>(args)...);
			};
			std::visit(exitVisitor, m_varStates);

			m_varStates.emplace<TargetState>();
			auto enterVisitor = [&](auto& state)
			{
				obj.OnEnterState(state, std::forward<Args>(args)...);
			};

			std::visit(enterVisitor, m_varStates);

		}

	private:
		std::variant<States...>  m_varStates;
	};

	// With the help of a "selector" function, the invoke helper
	// is able to select one of an ovelroaded set of member functions
	template<template <typename ... Args> typename FSelector,
			 typename Obj>
	class InvokeHelper
	{
	public:
		InvokeHelper(Obj& obj)
			:m_obj(obj)
		{ }

		template<typename ... Args>
		void operator()(Args&& ... args)
		{
			auto fToCall = FSelector<Args...>::Select();
			(m_obj.*fToCall)(std::forward<Args>(args)...);
		}

	private:
		Obj&  m_obj;
	};

}