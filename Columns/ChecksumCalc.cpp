#include "ChecksumCalc.h"

void geng::serial::ChecksumCalculator::Seed(unsigned long long checksumSeed)
{
	m_generator.seed(checksumSeed);
}

bool geng::serial::ChecksumCalculator::UpdateChecksum(const void* pBuff, size_t byteCount)
{
	if (byteCount == 0)
	{
		return true;
	}

	// First "paste" the odd bytes into the right position
	const uint8_t* pByteBuff = static_cast<const uint8_t*>(pBuff);

	if (m_leftoverCount > 0)
	{
		size_t numToRead = std::min(sizeof(TChecksumWord) - m_leftoverCount, byteCount);
		if (numToRead > 0)
		{
			uint8_t* poddDataBytes = reinterpret_cast<uint8_t*>(&m_oddData);
			memcpy(poddDataBytes + m_leftoverCount, pByteBuff, numToRead);
			pByteBuff += numToRead;
			byteCount -= numToRead;
		}

		m_leftoverCount += numToRead;

		if (m_leftoverCount == sizeof(TChecksumWord))
		{
			AddToChecksum(m_oddData);
			m_leftoverCount = 0;
		}

	}

	// If m_leftoverCount is *not* 0, then byteCount *is* 0
	while (byteCount >= sizeof(TChecksumWord))
	{
		AddToChecksum(*(reinterpret_cast<const TChecksumWord*>(pByteBuff)));
		pByteBuff += sizeof(TChecksumWord);
		byteCount -= sizeof(TChecksumWord);
	}

	if (byteCount > 0)
	{
		m_oddData = 0;
		uint8_t* poddDataBytes = reinterpret_cast<uint8_t*>(&m_oddData);
		memcpy(poddDataBytes, pByteBuff, byteCount);
		m_leftoverCount = byteCount;
	}

	return true;
}

void geng::serial::ChecksumCalculator::AddToChecksum(TChecksumWord qword)
{
	TChecksumWord genIndex = m_generator();
	m_runningChecksum += qword * genIndex;
}

geng::serial::TChecksumWord geng::serial::ChecksumCalculator::FinalizeChecksum()
{
	// Complete the last part of the checksum if it's missing
	if (m_leftoverCount > 0)
	{
		AddToChecksum(m_oddData);
		m_leftoverCount = 0;
	}

	return m_runningChecksum;
}