#pragma once

#include "IGame.h"

#include <string>
#include <type_traits>

namespace geng
{
	
	template<typename Interface>
	class TemplatedGameComponent : public Interface
	{
	public:
		static_assert(std::is_base_of_v<IGameComponent, Interface>, "TemplatedGameComponent Interface: must derive from IGameComponent");

		const char* GetName() const override
		{
			return m_name.c_str();
		}

		bool Initialize(const std::shared_ptr<IGame>& pGame) override
		{
			return true;
		}
		
		void WindDown(const std::shared_ptr<IGame>& pGame) override
		{ }

	protected:
		TemplatedGameComponent(const char* pName)
			:m_name(pName)
		{ }
	private:
		std::string m_name;
	};

	using BaseGameComponent = TemplatedGameComponent<IGameComponent>;

	enum class GetComponentResult
	{
		OK,
		NoComponent,
		ComponentWrongType
	};

	template<typename T>
	std::shared_ptr<T> GetComponentAs(IGame* pGame, const char* pName, GetComponentResult& result)
	{
		result = GetComponentResult::OK;

		std::shared_ptr<T> pTypedComponent{};

		auto pComponent = pGame->GetComponent(pName);
		if (!pComponent)
		{
			result = GetComponentResult::NoComponent;
			return pTypedComponent;
		}

		pTypedComponent = std::static_pointer_cast<T>(pComponent);
		if (!pTypedComponent)
		{
			result = GetComponentResult::ComponentWrongType;
		}

		return pTypedComponent;
	}

	template<typename T>
	std::shared_ptr<T> GetComponentAs(IGame* pGame, const char* pName)
	{
		GetComponentResult gcr;
		return GetComponentAs<T>(pGame, pName, gcr);
	}
}