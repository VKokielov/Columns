#include "Filestream.h"

bool geng::serial::FileReadStream::CanRead(size_t byteCount)
{
	if (!IsValid())
	{
		return false;
	}

	return true;
}

size_t geng::serial::FileReadStream::Read(void* pBuff, size_t byteCount)
{
	if (!IsValid())
	{
		return 0;
	}

	// Read
	size_t nRead = fread(pBuff, sizeof(uint8_t), byteCount, GetFile());
	
	SetValid(nRead == byteCount);
	return nRead;
}

bool geng::serial::FileWriteStream::CanWrite(size_t byteCount)
{
	if (!IsValid())
	{
		return false;
	}

	return true;
}

size_t geng::serial::FileWriteStream::Write(const void* pBuff, size_t byteCount)
{
	if (!IsValid())
	{
		return 0;
	}

	size_t nWritten = fwrite(pBuff, sizeof(uint8_t), byteCount, GetFile());

	SetValid(nWritten == byteCount);
	return nWritten;
}

bool geng::serial::FileWriteStream::Flush()
{
	auto flushRet = fflush(GetFile());

	return flushRet == 0;
}