#pragma once

#include "UIBase.h"
#include "IDataTree.h"

#include <string>

namespace geng::ui
{
	class UICommand;

	enum class ElementResult
	{
		OK,
		InvalidElement,
		InvalidParent,
		SubmoduleError
	};

	struct ElementClassInfo
	{
		UIClassDef classDef;
		ElementClassID classId;
	};

	struct ElementCreateRequest
	{
		// [in]
		ElementClassID classId;
		ElementID parentId;
		const data::IDatum* elementArgs;
		// [out]
		ElementResult out_createResult;
		ElementID out_createdId;
		std::string* out_errorText{ nullptr };
	};

	struct ElementDestroyRequest
	{
		// [in]
		ElementID elementId;
		// [out]
		ElementResult out_destroyResult;
	};

	struct ElementCommandRequest
	{
		// [in]
		ElementID elementId;
		AgentID agentId;
		const data::IDatum* command{ nullptr };
		// [out]
		ElementResult out_commandResult;
		std::string* out_errorText{ nullptr };
	};

	// All commands for the UI are handled here, including structural commands
	// like Create, Destroy, SetParent... -- and commands 

	class IUICommandManager : public IUIManager
	{
	public:
		// Find an element class by name
		virtual const ElementClassInfo* GetClass(const char* pClassName) = 0;
		// Create an element or multiple elements
		virtual bool CreateElements(ElementCreateRequest* pRequests,
			size_t nCount) = 0;
		// Destroy an element or multiple elements
		virtual bool DestroyElements(ElementDestroyRequest* pRequests,
			size_t nCount) = 0;

		// Issue a command to an element
		virtual bool IssueCommands(ElementCommandRequest* pRequests,
			size_t nCount) = 0;
		
	};

}