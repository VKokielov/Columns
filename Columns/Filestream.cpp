#include "Filestream.h"
#include "Packet.h"

// FileReadStream
geng::serial::FileReadStream::FileReadStream(FileUPtr&& pFile, const FileStreamHeader* pHeader)
	:FileStreamBase<IReadStream>(std::move(pFile), pHeader)
{
	// NOTE:  If the file has a header, the pHeader entry *must not* be null!
	// Otherwise the header is treated as data!
	if (FileStreamBase<IReadStream>::HasHeader())
	{
		if (!ProcessHeader())
		{
			return;
		}
	}

	SetValid(true);
}

bool geng::serial::FileReadStream::ProcessHeader()
{
	if (GetFile() == nullptr)
	{
		m_checkResult = FileValidityCheckResult::NoFile;
		return false;
	}

	m_checkResult = FileValidityCheckResult::OK;

	const FileStreamHeader& hdrSettings = FileStreamBase<IReadStream>::GetHeader();

	// Read and compare the signature string
	if (!hdrSettings.headerConstant.empty())
	{
		using THeaderChar = decltype(hdrSettings.headerConstant)::value_type;
		auto hdrLength = hdrSettings.headerConstant.size();

		std::unique_ptr<THeaderChar[]>
			pSigBuffer{ new THeaderChar[hdrSettings.headerConstant.size()] };
		
		// First read...
		if (fread(pSigBuffer.get(),
			sizeof(THeaderChar),
			hdrLength,
			GetFile()) != hdrLength)
		{
			m_checkResult = FileValidityCheckResult::HeaderError;
			return false;
		}

		// Compare the read-out bytes
		if (hdrSettings.headerConstant.compare(0, hdrLength, pSigBuffer.get(), hdrLength) != 0)
		{
			m_checkResult = FileValidityCheckResult::SignatureMismatch;
			return false;
		}
	}

	// Read in the version (always present in the header)
	if (fread(&m_version, sizeof(TFormatVersion), 1, GetFile())
		< 1)
	{
		m_checkResult = FileValidityCheckResult::HeaderError;
		return false;
	}

	// Checksum
	if (hdrSettings.hasChecksum)
	{
		// Try to get the checksum, and then compute it over the file
		// if it's here -- and verify
		ChecksumRecord  csRecord{ false, 0 };

		if (fread(&csRecord, sizeof(csRecord), 1, GetFile()) != sizeof(csRecord))
		{
			m_checkResult = FileValidityCheckResult::HeaderError;
			return false;
		}

		if (!csRecord.hasChecksum)
		{
			m_checkResult = FileValidityCheckResult::NoChecksumWritten;
			return false;
		}

		// Compute the complete checksum and verify it
		TChecksumCoefficientGenerator coeffGenerator;
		long curPos = ftell(GetFile());
		bool hasData{ true };

		coeffGenerator.seed(hdrSettings.checksumSeed);

		TChecksumWord runningChecksum{ 0 };
		while (hasData)
		{
			TChecksumWord curWord{ 0 };
			size_t readBytes = fread(&curWord, sizeof(uint8_t), sizeof(TChecksumWord),
				GetFile());

			hasData = readBytes == sizeof(TChecksumWord);

			if (readBytes > 0)
			{
				runningChecksum += coeffGenerator() * curWord;
			}
		}

		if (runningChecksum != csRecord.checksumVal)
		{
			m_checkResult = FileValidityCheckResult::ChecksumError;
		}
	}

	return true;
}

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

// FileWriteStream
geng::serial::FileWriteStream::FileWriteStream(FileUPtr&& pFile, const FileStreamHeader* pHeader)
:FileStreamBase<IWriteStream>(std::move(pFile), pHeader)
{
	if (FileStreamBase<IWriteStream>::HasHeader())
	{
		const FileStreamHeader& hdrSettings = FileStreamBase<IWriteStream>::GetHeader();
		if (hdrSettings.hasChecksum)
		{
			m_computeChecksum = true;
			m_generator.emplace();
			m_generator->seed(hdrSettings.checksumSeed);
		}
	}

	SetValid(true);
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

	if (m_computeChecksum)
	{
		if (!UpdateChecksum(pBuff, byteCount))
		{
			return 0;
		}
	}

	size_t nWritten = fwrite(pBuff, sizeof(uint8_t), byteCount, GetFile());

	SetValid(nWritten == byteCount);
	return nWritten;
}

bool geng::serial::FileWriteStream::WriteHeader()
{
	if (FileStreamBase<IWriteStream>::HasHeader())
	{
		// Write the header to the beginning of the file, and save the position of the checksum
		const FileStreamHeader& hdrSettings = FileStreamBase<IWriteStream>::GetHeader();

		if (ftell(GetFile()) != 0)
		{
			return false;
		}

		// Write the signature string
		if (!hdrSettings.headerConstant.empty())
		{
			using THeaderChar = decltype(hdrSettings.headerConstant)::value_type;
			auto hdrLength = hdrSettings.headerConstant.size();

			if (fwrite(hdrSettings.headerConstant.data(),
				sizeof(THeaderChar),
				hdrLength,
				GetFile()) != hdrLength)
			{
				return false;
			}
		}

		// Write the version number
		if (fwrite(&hdrSettings.versionNo, sizeof(hdrSettings.versionNo), 1, GetFile())
			!= sizeof(hdrSettings.versionNo))
		{
			return false;
		}

		// Save the position of the checksum to overwrite it later
		m_checksumPos = ftell(GetFile());

		static ChecksumRecord blankChecksum{};
		if (fwrite(&blankChecksum, sizeof(blankChecksum), 1, GetFile())
			!= sizeof(blankChecksum))
		{
			return false;
		}

		return true;
	}

	return false;
}

void geng::serial::FileWriteStream::AddToChecksum(TChecksumWord qword)
{
	if (!m_computeChecksum)
	{
		return;
	}

	TChecksumWord genIndex = (*m_generator)();
	m_runningChecksum += qword * genIndex;
}

bool geng::serial::FileWriteStream::UpdateChecksum(const void* pBuff, size_t byteCount)
{
	// NOTE:  If byteCount == 0, pBuff is ignored (never read)

	// First "paste" the odd bytes into the right position
	const uint8_t* pByteBuff = static_cast<const uint8_t*>(pBuff);
	if (m_leftoverCount > 0)
	{
		TChecksumWord startOddBytes{ 0 };

		size_t numToRead = std::min(sizeof(uint64_t) - m_checksumPos, byteCount);
		if (numToRead > 0)
		{
			memcpy(&startOddBytes, pBuff, numToRead);
			pByteBuff += numToRead;
			byteCount -= numToRead;
		}

		TChecksumWord csEntry = m_oddData | startOddBytes;
		AddToChecksum(csEntry);
		m_leftoverCount = 0;
	}

	while (byteCount >= sizeof(TChecksumWord))
	{
		AddToChecksum(*(reinterpret_cast<const TChecksumWord*>(pByteBuff)));
		pByteBuff += sizeof(TChecksumWord);
		byteCount -= sizeof(TChecksumWord);
	}

	if (byteCount > 0)
	{
		m_oddData = 0;
		memcpy(&m_oddData, pByteBuff, byteCount);
	}

	m_leftoverCount = byteCount;
	return true;
}

bool geng::serial::FileWriteStream::WriteChecksum()
{
	if (!m_computeChecksum)
	{
		return false;
	}

	// Complete the last part of the checksum if it's missing
	if (m_leftoverCount > 0)
	{
		AddToChecksum(m_oddData);
	}

	// Go to the beginning, where the checksum needs to be recorded
	if (fseek(GetFile(), m_checksumPos, SEEK_SET) < 0)
	{
		return false;
	}

	// Record the checksum
	ChecksumRecord csRecord{ true, m_runningChecksum };
	if (fwrite(&csRecord, sizeof(csRecord), 1, GetFile())
		!= sizeof(csRecord))
	{
		return false;
	}

	return true;
}

bool geng::serial::FileWriteStream::Flush()
{
	auto flushRet = fflush(GetFile());

	return flushRet == 0;
}