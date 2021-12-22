#pragma once

#include "FileUtils.h"
#include "Bytestream.h"
#include "ChecksumCalc.h"
#include <optional>
#include <string>
#include <random>

namespace geng::serial
{


	struct FileStreamHeader
	{
		std::string headerConstant;
		TFormatVersion versionNo;
		bool hasChecksum;
		unsigned long long checksumSeed;
	};

	enum class FileValidityCheckResult
	{
		OK,
		NoFile,
		HeaderError,
		SignatureMismatch,
		NoChecksumWritten,
		ChecksumError
	};

	template<typename I>
	class FileStreamBase : public I
	{
	public:
		FileUPtr Release()
		{
			return std::move(m_pFile);
		}

		TFormatVersion GetFormatVersion() const override
		{
			return m_version;
		}
	protected:
		FileStreamBase(FileUPtr&& pFile, const FileStreamHeader* pHeader)
			:m_pFile(std::move(pFile))
		{ 
			m_streamValid = !ferror(m_pFile.get()) && !feof(m_pFile.get());
			if (pHeader)
			{
				m_header.emplace(*pHeader);
			}
		}


		FILE* GetFile() {
			return m_pFile.get();
		}

		bool IsValid() const { return m_streamValid; }
		void SetValid(bool val)
		{
			m_streamValid = val;
		}

		bool HasHeader() const { return m_header.has_value(); }
		const FileStreamHeader& GetHeader() const
		{
			return m_header.value();
		}

		void SetFormatVersion(TFormatVersion version)
		{
			m_version = version;
		}
		
	private:
		FileUPtr  m_pFile;
		bool m_streamValid{ false };
		std::optional<FileStreamHeader>  m_header;
		TFormatVersion m_version{ 0 };
	};

	struct ChecksumRecord
	{
		bool hasChecksum{ false };
		TChecksumWord checksumVal{ 0 };
	};

	class FileReadStream : public FileStreamBase<IReadStream>
	{
	public:
		FileReadStream(FileUPtr&& pFile, const FileStreamHeader* pHeader = nullptr);

		FileValidityCheckResult GetCheckResult() const 
		{
			return m_checkResult;
		}

		// WARNING:  Due to the way C file IO works, this function always returns true
		bool CanRead(size_t byteCount) override;
		size_t Read(void* pBuff, size_t byteCount) override;

	private:
		bool ProcessHeader();

		FileValidityCheckResult m_checkResult{ FileValidityCheckResult::NoFile };
	};

	class FileWriteStream : public FileStreamBase<IWriteStream>
	{
	public:
		FileWriteStream(FileUPtr&& pFile, const FileStreamHeader* pHeader = nullptr);

		bool WriteHeader();
		// Should be called at the end of all write operations
		// Rewinds the file pointer to the beginning and 
		bool WriteChecksum(); 

		bool CanWrite(size_t byteCount) override;
		size_t Write(const void* pBuff, size_t byteCount) override;
		bool Flush() override;

	private:
		bool m_headerWritten{ false };
		long int m_checksumPos{ 0 };

		std::optional<ChecksumCalculator> m_checksumCalc;
	};

}