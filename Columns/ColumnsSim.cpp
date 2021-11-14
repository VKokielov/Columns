#include "ColumnsSim.h"

void geng::columns::ColumnsSim::AddSetFromPoint(bool axisEnabled,
	Axis axis,
	const Point& origin,
	unsigned long minSize)
{
	if (axisEnabled)
	{
		std::vector<Point>  ptSet;
		auto fillPoints = [&ptSet](const Point& member)
		{
			ptSet.emplace_back(member);
			return true;
		};

		IterateAxis(origin, Axis::Horizontal, fillPoints);
		if (ptSet.size() >= minSize)
		{
			m_setsToScan.emplace_back(std::move(ptSet));
		}
	}
}

void geng::columns::ColumnsSim::GenerateGridSets()
{
	// Precompute the sets that 
	// Origin - 0,m_overflow - generate horizontal, vertical, downsloping set
	// Top row - x,m_overflow : x >= 1 -- generate vertical, upsloping, downsloping set
	// Left column 0,y : y >= 3 -- generate horizontal, downsloping set
	// Right column X-1,y : y >= 3 -- generate upsloping set

	bool genH{ false };
	bool genV{ false };
	bool genD{ false };
	bool genU{ false };

	auto genSetAtPoint = [this,&genH, &genV, &genD, &genU](const Point& origin)
	{
		AddSetFromPoint(genH, Axis::Horizontal, origin, 2);
		AddSetFromPoint(genV, Axis::Vertical, origin, 2);
		AddSetFromPoint(genD, Axis::Downslope, origin, 2);
		AddSetFromPoint(genU, Axis::Upslope, origin, 2);
		return true;
	};

	// Origin
	Point topCorner{ 0, m_overflow};
	AddSetFromPoint(true, Axis::Horizontal,topCorner, 2);
	AddSetFromPoint(true, Axis::Vertical, topCorner, 2);
	AddSetFromPoint(true, Axis::Downslope, topCorner, 2);

	// Top row
	genV = true;
	genU = true;
	genD = true;
	IterateAxis(topCorner, Axis::Horizontal, genSetAtPoint);

	// Left column
	++topCorner.y;

	genH = true;
	genU = false;
	genV = false;

	IterateAxis(topCorner, Axis::Vertical, genSetAtPoint);

	// Right column
	topCorner.x = m_size.x - 1;
	genH = false;
	genD = false;
	genU = true;
	IterateAxis(topCorner, Axis::Vertical, genSetAtPoint);

	// The vector of vectors should now be full with all the sets to check
	// to find which squares need to be removed
}

void geng::columns::ColumnsSim::MarkRemovables(const std::vector<Point>& scanSet,
	size_t start,
	size_t end,
	unsigned int count)
{
	if (end - 1 - start >= count && IsRemovable(GetContents(scanSet[start])))
	{
		// Mark all points from "start" to "end" as removable
		for (size_t i = start; i < end; ++i)
		{
			const Point& toRemove = scanSet[i];
			m_toRemove.emplace(PointToIndex(toRemove));
			m_columnsToCompact.emplace(toRemove.x);
		}
	}
}

void geng::columns::ColumnsSim::ComputeRemovablesInSet(const std::vector<Point>& scanSet, unsigned int count)
{
	// Find count matching removable points in a row
	size_t seqStart = 0;
	size_t seqCur = 0;

	while (seqCur < scanSet.size())
	{
		if (GetContents(scanSet[seqStart]) != GetContents(scanSet[seqCur]))
		{
			MarkRemovables(scanSet, seqStart, seqCur, count);
			seqStart = seqCur;
		}
		++seqCur;
	}

	// Last stretch
	MarkRemovables(scanSet, seqStart, seqCur, count);
}

void geng::columns::ColumnsSim::ComputeRemovables(unsigned int count)
{
	// Go through each stored set and scan it to find removables
	m_columnsToCompact.clear();
	m_toRemove.clear();

	for (size_t iSet = 0; iSet < m_setsToScan.size(); ++iSet)
	{
		ComputeRemovablesInSet(m_setsToScan[iSet], count);
	}
}

void geng::columns::ColumnsSim::CompactColumn(unsigned int x)
{
	// This is just the partition algorithm applied to blank grid squares vs stones
	// in effect, the nonblanks all end up on the bottom

	unsigned int readY = m_size.y - 1;
	unsigned int writeY = readY;
	while (readY >= m_overflow)
	{
		Point readPt{ x,readY };
		Point writePt{ x, writeY};

		if (IsBlank(GetContents(readPt)))
		{
			// Do not disturb the write pointer
			--readY;
		}
		else
		{
			if (readY < writeY)
			{
				// Move the stone from nonblank to blank and update both 
				MoveBlock(readPt, writePt);
			}
			--readY;
			--writeY;
		}
	}
}

void geng::columns::ColumnsSim::CompactColumns()
{
	for (unsigned int xCol : m_columnsToCompact)
	{
		CompactColumn(xCol);
	}
}

bool geng::columns::ColumnsSim::CanPlayerColumnBeOriented(const PlayerSet& playerColumn, bool isH)
{
	unsigned int wingSize = playerColumn.WingSize();

	unsigned int startingCoord = isH ? playerColumn.locCenter.x : playerColumn.locCenter.y;
	if (startingCoord < wingSize)
	{
		return false;
	}

	startingCoord -= wingSize;

	unsigned int pieceLen = 2 * wingSize + 1;

	// All entries must be blank
	Point startingPoint{ playerColumn.locCenter };

	if (isH)
	{
		startingPoint.x = startingCoord;
	}
	else
	{
		startingPoint.y = startingCoord;
	}
	
	unsigned int numIterated{ 0 };

	auto checkRoom = [&numIterated, pieceLen, this](const Point& pt)
	{
		if (numIterated == pieceLen || !IsBlank(GetContents(pt)))
		{
			return false;
		}
		++numIterated;
		return true;
	};

	IterateAxis(startingPoint, isH ? Axis::Horizontal : Axis::Vertical, checkRoom);

	return numIterated == pieceLen;
}
