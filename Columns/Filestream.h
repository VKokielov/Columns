#pragma once

#include "FileUtils.h"
#include "Bytestream.h"

namespace geng::serial
{
	template<typename I>
	class FileStreamBase : public I
	{
	public:
		FileUPtr Release()
		{
			return std::move(m_pFile);
		}
	protected:
		FileStreamBase(FileUPtr&& pFile)
			:m_pFile(std::move(pFile))
		{ 
			m_streamValid = !ferror(m_pFile.get()) && !feof(m_pFile.get());
		}


		FILE* GetFile() {
			return m_pFile.get();
		}

		bool IsValid() const { return m_streamValid; }
		void SetValid(bool val)
		{
			m_streamValid = val;
		}
	private:
		FileUPtr  m_pFile;
		bool m_streamValid{ false };
	};

	class FileReadStream : public FileStreamBase<IReadStream>
	{
	public:
		FileReadStream(FileUPtr&& pFile)
			:FileStreamBase<IReadStream>(std::move(pFile))
		{}

		// WARNING:  Due to the way C file IO works, this function always returns true
		bool CanRead(size_t byteCount) override;
		size_t Read(void* pBuff, size_t byteCount) override;

	};

	class FileWriteStream : public FileStreamBase<IWriteStream>
	{
	public:
		FileWriteStream(FileUPtr&& pFile)
			:FileStreamBase<IWriteStream>(std::move(pFile))
		{}

		bool CanWrite(size_t byteCount) override;
		size_t Write(const void* pBuff, size_t byteCount) override;
	};

}