#include "ColumnsData.h"

bool geng::serial::EncodeData(serial::IWriteStream* pWriteStream, const columns::Point& point)
{
	if (!EncodeData(pWriteStream, point.x))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, point.y))
	{
		return false;
	}

	return true;
}


bool geng::serial::EncodeData(serial::IWriteStream* pWriteStream, const columns::SimArgs& simArgs)
{

	if (!EncodeData(pWriteStream, simArgs.boardSize))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, simArgs.columnSize))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, simArgs.dropMilliseconds))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, simArgs.flashMilliseconds))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, simArgs.flashCount))
	{
		return false;
	}

	if (!EncodeData(pWriteStream, simArgs.randomSeed))
	{
		return false;
	}

	return true;
}


bool geng::serial::DecodeData(serial::IReadStream* pReadStream, columns::Point& point)
{
	if (!DecodeData(pReadStream, point.x))
	{
		return false;
	}

	if (!DecodeData(pReadStream, point.y))
	{
		return false;
	}

	return true;
}

bool geng::serial::DecodeData(serial::IReadStream* pReadStream, columns::SimArgs& simArgs)
{
	if (!DecodeData(pReadStream, simArgs.boardSize))
	{
		return false;
	}

	if (!DecodeData(pReadStream, simArgs.columnSize))
	{
		return false;
	}

	if (!DecodeData(pReadStream, simArgs.dropMilliseconds))
	{
		return false;
	}

	if (!DecodeData(pReadStream, simArgs.flashMilliseconds))
	{
		return false;
	}

	if (!DecodeData(pReadStream, simArgs.flashCount))
	{
		return false;
	}

	if (!DecodeData(pReadStream, simArgs.randomSeed))
	{
		return false;
	}

	return true;
}