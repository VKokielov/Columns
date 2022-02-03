#pragma once

#include "UIImplDatumBase.h"
#include <type_traits>

namespace geng::ui::impl
{

	template<typename T>
	class UIBaseClassDatumValue : public 
		UIDatumBase<
					std::conditional_t<std::is_same_v<T,std::string>,
							IUIStringDatum,
							IUITypedPrimitiveDatum<T> >
	{
	public:
		data::ElementType GetPrimitiveType() const override
		{
			if constexpr (std::is_same_v<bool, T>)
			{
				return data::ElementType::Boolean;
			}
			else if constexpr (std::is_same_v<int8_t, T>)
			{
				return data::ElementType::Int8;
			}
			else if constexpr (std::is_same_v<uint8_t, T>)
			{
				return data::ElementType::UInt8;
			}
			else if constexpr (std::is_same_v<int16_t, T>)
			{
				return data::ElementType::Int16;
			}
			else if constexpr (std::is_same_v<uint16_t, T>)
			{
				return data::ElementType::UInt16;
			}
			else if constexpr (std::is_same_v<int32_t, T>)
			{
				return data::ElementType::Int32;
			}
			else if constexpr (std::is_same_v<uint32_t, T>)
			{
				return data::ElementType::UInt32;
			}
			else if constexpr (std::is_same_v<int64_t, T>)
			{
				return data::ElementType::Int32;
			}
			else if constexpr (std::is_same_v<uint64_t, T>)
			{
				return data::ElementType::UInt32;
			}
			else if constexpr (std::is_same_v<float, T>)
			{
				return data::ElementType::Float;
			}
			else if constexpr (std::is_same_v<double, T>)
			{
				return data::ElementType::Double;
			}
			else if constexpr (std::is_same_v<std::string, T>)
			{
				return data::ElementType::String;
			}
			else
			{
				static_assert(true, "Unsupported value data type for UI value datum");
			}
		}

		auto
			GetData() const override
		{
			return GetValue();
		}

	protected:
		using UIDatumBase<IUIElementDatum>::UIDatumBase<IUIElementDatum>;

		// "Known" interface, normally to be used by the derived class
		// but can also be "promoted" directly

		auto GetValue() const 
		{ 
			if constexpr (std::is_same_v<T, std::string>)
			{
				return m_value.c_str();
			}
			else
			{
				return m_value;
			}
		}

		// Setters
		template<typename U>
		void SetValue(U&& newValue)
		{
			m_value = newValue;
		}

		template<typename F>
		void SetValueWithCallback(F&& fSetter)
		{
			fSetter(m_value);
		}

	private:
		T m_value;
	};

	// This class promotes visibility of certain base class members, allowing
	// the above class to be used directly
	template<typename T>
	class UIDatumValue : public UIBaseClassDatumValue<T>
	{
	public:
		using UIBaseClassDatumValue<T>::UIBaseClassDatumValue<T>;
		using UIBaseClassDatumValue<T>::GetValue;
		using UIBaseClassDatumValue<T>::SetValue;

		template<typename F>
		void SetValueWithCallback(F&& fSetter)
		{
			UIBaseClassDatumValue<T>::SetValueWithCallback(m_value);
		}

	};
}