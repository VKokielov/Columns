#include "BaseGameComponent.h"

const char* geng::BaseGameComponent::GetName() const
{
	return m_name.c_str();
}
geng::GameComponentType geng::BaseGameComponent::GetType() const
{
	return m_type;
}

bool geng::BaseGameComponent::Initialize(const std::shared_ptr<IGame>& pGame)
{
	return true;
}
void geng::BaseGameComponent::WindDown(const std::shared_ptr<IGame>& pGame)
{

}