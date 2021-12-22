#pragma once

#include <cinttypes>

namespace geng::serial
{

	using TFormatVersion = uint32_t;

	class IReadStream
	{
	public:
		virtual ~IReadStream() = default;
		virtual TFormatVersion GetFormatVersion() const = 0;
		virtual bool CanRead(size_t byteCount) = 0;
		virtual size_t Read(void* pBuff, size_t byteCount) = 0;
	};

	class IWriteStream
	{
	public:
		virtual ~IWriteStream() = default;
		virtual TFormatVersion GetFormatVersion() const = 0;
		virtual bool CanWrite(size_t byteCount) = 0;
		virtual size_t Write(const void* pBuff, size_t byteCount) = 0;
		virtual bool Flush() = 0;
	};


}