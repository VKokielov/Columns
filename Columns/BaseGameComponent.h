#pragma once

#include "IGame.h"

#include <string>

namespace geng
{

	class BaseGameComponent : public IGameComponent
	{
	public:
		const char* GetName() const override;
		GameComponentType GetType() const override;

		bool Initialize(const std::shared_ptr<IGame>& pGame) override;
		void WindDown(const std::shared_ptr<IGame>& pGame) override;
	protected:
		BaseGameComponent(const char* pName, GameComponentType type)
			:m_name(pName),
			m_type(type)
		{ }
	private:
		std::string m_name;
		GameComponentType m_type;
	};

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

		auto pComponent = pGame->GetComponent(pName);
		if (!pComponent)
		{
			result = GetComponentResult::NoComponent;
			return pComponent;
		}

		auto pTypedComponent = std::dynamic_pointer_cast<T>(pComponent);
		if (!pTypedComponent)
		{
			result = GetComponentResult::ComponentWrongType;
		}

		return pTypedComponent;
	}
}