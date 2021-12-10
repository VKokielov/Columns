#pragma once

#include <cinttypes>

namespace geng::serial
{

	class IReadStream
	{
	public:
		virtual ~IReadStream() = default;
		virtual bool CanRead(size_t byteCount) = 0;
		virtual size_t Read(void* pBuff, size_t byteCount) = 0;
	};

	class IWriteStream
	{
	public:
		virtual ~IWriteStream() = default;
		virtual bool CanWrite(size_t byteCount) = 0;
		virtual size_t Write(const void* pBuff, size_t byteCount) = 0;
		virtual bool Flush() = 0;
	};


}