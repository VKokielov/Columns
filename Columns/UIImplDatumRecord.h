#pragma once

#include "UIImplDatumBase.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <initializer_list>
#include <string_view>
#include <stdexcept>
#include <array>

namespace geng::ui::impl
{
	// A "record" is like a struct: a dictionary of named members
	// Since the names are known beforehand, the mapping is 
	// constructed only once per class.  (It consists of two parts -- one constexpr,
	// one made at runtime from the constexpr).

	template<std::size_t N>
	class UIBaseClassRecordDef
	{
	public:
		constexpr UIBaseClassRecordDef(const std::string_view (&fieldNames)[N])
		{
			for (std::size_t i = 0; i < N; ++i)
			{
				for (std::size_t j = 0; j < i; ++j)
				{
					if (fieldNames[i] == m_def[j])
					{
						throw std::runtime_error("field name duplicate in base class record definition");
					}
				}
				m_def[i] = fieldNames[i];
			}
		}

		constexpr std::size_t IndexOf(const char* fieldName) const
		{
			for (std::size_t i = 0; i < N; ++i)
			{
				if (m_def[i] == fieldName)
				{
					return i;
				}
			}

			throw std::runtime_error("Field with given name not found");
		}

		constexpr const char* GetFieldName(std::size_t idx) const
		{
			if (idx >= N)
			{
				throw std::runtime_error("Invalid field index");
			}

			return m_def[idx].c_str();
		}

	private:
		std::string_view m_def[N]{};
	};

	class UIBaseClassRecordRuntimeDef
	{
	public:
		template<std::size_t N>
		UIBaseClassRecordRuntimeDef(const UIBaseClassRecordDef<N>& recordDef)
		{
			for (std::size_t i = 0; i < N; ++i)
			{
				const char* fieldNameC = recordDef.GetFieldName(i);
				std::string fieldName(fieldNameC);

				m_fieldVec.emplace_back(fieldName);
				m_fieldMap.emplace(fieldName, i);
			}
		}

		std::size_t GetSize() const { return m_fieldVec.size(); }

		bool GetEntry(const char* pFieldName, std::size_t& idx) const
		{
			auto itField = m_fieldMap.find(pFieldName);
			if (itField != m_fieldMap.end())
			{
				idx = itField->second;
				return true;
			}

			return false;
		}

		template<typename F>
		void Iterate(F&& callback) const
		{
			for (const auto& rField : m_fieldVec)
			{
				if (!callback(rField))
				{
					return;
				}
			}
		}

	private:
		std::unordered_map<std::string, size_t>  m_fieldMap;
		std::vector<std::string> m_fieldVec;
	};


	class UIBaseClassDatumRecord : public UIDatumBase<IUIDictionaryDatum>
	{
	public:
		UIBaseClassDatumRecord(UIElementBase* pOwner,
			ElementID elementId,
			const UIBaseClassRecordRuntimeDef* pDefinition);
		
		const IUIDatum* GetEntry(const char* pKey) const override;
		bool Iterate(IUIDictCallback& rCallback) const override;
	protected:
		bool AddField(std::size_t fieldIdx, const IUIDatum* pDatum);
		
	private:
		std::vector<const geng::ui::IUIDatum*>   m_children;
		const UIBaseClassRecordRuntimeDef* m_pRuntimeDef;
	};

}