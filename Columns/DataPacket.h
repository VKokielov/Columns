#pragma once

#include "Packet.h"

namespace geng::serial
{
	// An implementation of IPacket representing a single piece of data for which EncodeData<>
	// has been defined and works correctly

	template<typename T>
	class DataPacket : public IPacket
	{
	public:
		DataPacket()
			:m_val()
		{ }

		DataPacket(const T& val)
			:m_val(val)
		{ }

		DataPacket(T&& val)
			:m_val(std::move(val))
		{ }

		const T& Get() const { return m_val; }
		T& Get() { return m_val; }

		bool Write(IWriteStream* pStream) override
		{
			return EncodeData(pStream, m_val);
		}
		
		bool Read(IReadStream* pStream) override
		{
			return DecodeData(pStream, m_val);
		}

	private:
		T  m_val;
	};

}