#pragma once

#include "Bytestream.h"

namespace geng::serial
{
	template<typename Data>
	bool EncodeData(serial::IWriteStream* pWriteStream, const Data& data)
	{
		return pWriteStream->Write(&data, sizeof(Data)) == sizeof(Data);
	}

	template<typename Data>
	bool DecodeData(serial::IReadStream* pReadStream, Data& data)
	{
		return pReadStream->Read(&data, sizeof(Data)) == sizeof(Data);
	}

	class IPacket
	{
	public:
		virtual ~IPacket() = default;
		virtual bool Write(IWriteStream* pStream) = 0;
		virtual bool Read(IReadStream* pStream) = 0;
	};
}