#pragma once

#include <memory>

namespace geng
{

	enum class GameComponentType
	{
		SetUp,
		Simulation,
		IO
	};
	
	class IGame;
	class IFrameListener;

	class IGameComponent
	{
	public:
		virtual const char* GetName() const = 0;
		virtual GameComponentType GetType() const = 0;
		virtual ~IGameComponent() = default;

		virtual bool Initialize(const std::shared_ptr<IGame>& pGame) = 0;
		virtual void WindDown(const std::shared_ptr<IGame>& pGame) = 0;

		virtual IFrameListener* GetFrameListener() = 0;
	};

	class IGame
	{
	public:
		virtual ~IGame() = default;

		virtual bool AddComponent(const std::shared_ptr<IGameComponent>& pComponent) = 0;
		virtual const std::shared_ptr<IGameComponent>&
			GetComponent(const char* pName) = 0;

		virtual void LogError(const char* pError) = 0;

		virtual bool Run() = 0;
	};

}