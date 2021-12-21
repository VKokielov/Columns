#pragma once

#include <random>

namespace geng::serial
{
	using TChecksumCoefficientGenerator = std::mt19937_64;
	using TChecksumWord = uint64_t;

	class ChecksumCalculator
	{
	public:
		void Seed(unsigned long long checksumSeed);

		bool UpdateChecksum(const void* pBuff, size_t byteCount);
		void AddToChecksum(TChecksumWord qword);

		TChecksumWord FinalizeChecksum();

	private:
		TChecksumWord m_oddData{ 0 };
		size_t m_leftoverCount{ 0 };
		TChecksumWord m_runningChecksum{ 0 };
		TChecksumCoefficientGenerator m_generator;
	};

}