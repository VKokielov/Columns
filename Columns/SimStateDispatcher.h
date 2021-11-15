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

		template<typename Obj, typename ... Args>
		void Dispatch(Obj& obj, Args&&...args)
		{
			// Generic lambda for a series of states
			auto visitor = [&](const auto& state)
			{
				obj.OnState(state, 
					        static_cast<Derived&>(*this), 
					        std::forward<Args>(args)...);
			};

			std::visit(visitor, m_varStates);
		}

		template<typename TargetState>
		void Transition()
		{
			m_varStates.emplace<TargetState>();
		}

	private:
		std::variant<States...>  m_varStates;
	};
}