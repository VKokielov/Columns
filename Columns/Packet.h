#pragma once

#include "Bytestream.h"
#include <array>

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

	// Encode/decode a fixed size array as a black-box
	// Not a very fast solution; a specialization is provided for byte arrays, which are native
	// to the streams
	template<typename T, size_t N>
	bool EncodeData(serial::IWriteStream* pWriteStream, const std::array<T, N>& inData)
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (!EncodeData(pWriteStream, inData[i]))
			{
				return false;
			}
		}
		return true;
	}

	template<typename T, size_t N>
	bool DecodeData(serial::IReadStream* pReadStream, std::array<T, N>& outData)
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (!DecodeData(pReadStream, outData[i]))
			{
				return false;
			}
		}
		return true;
	}

	template<size_t N>
	bool EncodeData(serial::IWriteStream* pWriteStream, const std::array<uint8_t, N>& inData)
	{
		return pWriteStream->Write(inData.data(), N) == N;
	}

	template<size_t N>
	bool DecodeData(serial::IReadStream* pReadStream, std::array<uint8_t, N>& outData)
	{
		return pReadStream->Read(outData.data(), N) == N;
	}

	class IPacket
	{
	public:
		virtual ~IPacket() = default;
		virtual bool Write(IWriteStream* pStream) = 0;
		virtual bool Read(IReadStream* pStream) = 0;
	};
}