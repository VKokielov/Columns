#pragma once

#include <variant>

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

		template<typename Obj, typename ... Args>
		void Dispatch(Obj& obj, Args&&...args)
		{
			// Generic lambda for a series of states
			auto visitor = [&](const auto& state)
			{
				obj.OnState(state, 
					        std::forward<Args>(args)...);
			};

			std::visit(visitor, m_varStates);
		}

		template<typename TargetState>
		void Transition()
		{
			m_varStates.emplace<TargetState>();
		}

		template<typename TargetState, typename Obj, typename ... Args>
		void Transition(Obj& obj, Args&...args)
		{
			auto exitVisitor = [&](const auto& state)
			{
				obj.OnExitState(state,std::forward<Args>(args)...);
			};
			std::visit(exitVisitor, m_varStates);

			m_varStates.emplace<TargetState>();
			auto enterVisitor = [&](const auto& state)
			{
				obj.OnEnterState(state, std::forward<Args>(args)...);
			};

			std::visit(enterVisitor, m_varStates);

		}

	private:
		std::variant<States...>  m_varStates;
	};
}