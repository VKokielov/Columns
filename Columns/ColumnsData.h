#pragma once

#include <string>
#include <cinttypes>
#include "Packet.h"
#include "SerializedCommands.h"

namespace geng
{
	namespace columns
	{
		using RandomSeedType = unsigned long long;
		using ActionCommandID = size_t;

		struct Point
		{
			unsigned int x;
			unsigned int y;

			bool operator ==(const Point& rhs) const
			{
				return x == rhs.x && y == rhs.y;
			}

			bool operator ==(const Point& rhs)
			{
				return x == rhs.x && y == rhs.y;
			}
		};

		struct SimArgs
		{
			Point boardSize;
			unsigned int columnSize;
			unsigned int dropMilliseconds;
			unsigned int flashMilliseconds;
			unsigned int flashCount;
			RandomSeedType randomSeed;
		};

		struct InputArgs
		{
			PlaybackMode pbMode;
			std::string fileName;
			unsigned int userPlayer;
		};

		struct ColumnsArgs
		{
			InputArgs inputArgs;
			SimArgs simArgs;
		};
	}

	namespace serial
	{
		// Serialization helpers for Point and SimArgs
		// Although they duplicate the code, they are still better than pragma pack(1)
		bool EncodeData(serial::IWriteStream* pWriteStream, const columns::Point& point);
		bool EncodeData(serial::IWriteStream* pWriteStream, const columns::SimArgs& simArgs);

		bool DecodeData(serial::IReadStream* pReadStream, columns::Point& point);
		bool DecodeData(serial::IReadStream* pReadStream, columns::SimArgs& simArgs);
	}
	
}