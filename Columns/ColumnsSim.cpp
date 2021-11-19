#include "ColumnsSim.h"

unsigned int geng::columns::ColumnsSim::PointToIndex(const Point& at) const
{
	return at.x + m_size.x * at.y;
}

geng::columns::Point geng::columns::ColumnsSim::IndexToPoint(unsigned int idx) const
{
	return Point{ idx % m_size.x, idx / m_size.x };
}

geng::columns::GridContents geng::columns::ColumnsSim::GetContents(const Point& at, bool* pisvalid) const
{
	unsigned int idx = PointToIndex(at);
	if (pisvalid)
	{
		*pisvalid = false;
	}

	if (idx < m_gameGridSize)
	{
		return m_gameGrid[idx].contents;
		if (pisvalid)
		{
			*pisvalid = false;
		}
	}

	return EMPTY;
}

bool geng::columns::ColumnsSim::SetContents(const Point& at, GridContents contents)
{
	unsigned int idx = PointToIndex(at);
	if (idx < m_gameGridSize)
	{
		m_gameGrid[idx].contents = contents;
		return true;
	}

	return false;
}

bool geng::columns::ColumnsSim::MoveBlock(const Point& from, const Point& to)
{
	// Memberwise comparison
	if (from == to)
	{
		return true;
	}

	bool validGet;
	GridContents gcFrom = GetContents(from, &validGet);
	if (!validGet)
	{
		return false;
	}

	if (!SetContents(to, gcFrom))
	{
		return false;
	}

	SetContents(from, EMPTY);
	return true;
}

bool geng::columns::ColumnsSim::RemoveBlock(const Point& at)
{
	return SetContents(at, EMPTY);
}

bool geng::columns::ColumnsSim::IsValidShiftedPlayerColumn(const PlayerSet& target, 
	const PointDelta& delta) const
{
	// Is the new center location in range?
	if (target.CenterX() < (unsigned int)-delta.dx 
	|| target.CenterX() + delta.dx >= m_size.x
	|| target.CenterY() < (unsigned int)-delta.dy
	|| target.CenterY() + delta.dy >= m_size.y)
	{
		return false;
	}

	// New center
	Point newCenter{ target.CenterX() + delta.dx, target.CenterY() + delta.dy };
	unsigned int targetWingSize = target.WingSize();

	// Are the edges in range?
	if (target.isHorizontal)
	{
		if (newCenter.x < targetWingSize || newCenter.x + targetWingSize >= m_size.x)
		{
			return false;
		}
	}
	else
	{
		if (newCenter.y < target.WingSize() || newCenter.y + target.WingSize() >= m_size.y)
		{
			return false;
		}
	}

	// Check that all entries are either blank or in range of the source (current) player column
	// (Need to check the currenct column because it has not yet been removed)
	unsigned int colLen = target.Width();

	Point newStart{ newCenter.x, newCenter.y };
	target.isHorizontal ? newStart.x -= targetWingSize : newStart.y -= targetWingSize;

	unsigned int count{ 0 };

	while (count < colLen)
	{
		if (!m_playerColumn.InColumn(newStart)
			&& !IsBlank(GetContents(newStart)))
		{
			return false;
		}

		++count;
		target.isHorizontal ? ++newStart.x : ++newStart.y;
	}

	return true;
}

bool geng::columns::ColumnsSim::CanPlayerColumnShift(const PlayerSet& playerColumn, 
					bool isLeft)
{
	return IsValidShiftedPlayerColumn(playerColumn, PointDelta{ isLeft ? -1 : 1, 0 });
}

void geng::columns::ColumnsSim::RemovePlayerColumn(const PlayerSet& playerColumn)
{
	unsigned int colLen = playerColumn.Width();

	// Blank out the place where a player column is on the grid
	Point ptCol{ playerColumn.locCenter };
	if (playerColumn.isHorizontal)
	{
		ptCol.x -= playerColumn.WingSize();
	}
	else
	{
		ptCol.y -= playerColumn.WingSize();
	}

	unsigned int count = 0;
	while (count < colLen)
	{
		SetContents(ptCol, EMPTY);
		++count;
		playerColumn.isHorizontal ? ++ptCol.x : ++ptCol.y;
	}
}

void geng::columns::ColumnsSim::SetPlayerColumn(const PlayerSet& playerColumn)
{
	// This function "sets" the player column onto the board
	// No invariants are checked here.  The column is simply painted onto the 
	// board

	unsigned int colLen = playerColumn.Width();

	// Starting point in the color array
	// Always start with colLen at the beginning, since size_t is unsigned
	// Imagine the colors repeated three times: ABCABCABC.  We start with the second "A"
	// and adjust for orientation and shift.  The result is taken % the size of the array,
	// so it gives the expected result.

	size_t colorPos = colLen + playerColumn.startPt;

	Point ptCol{ playerColumn.locCenter };
	if (playerColumn.isHorizontal)
	{
		ptCol.x -= playerColumn.WingSize();
	}
	else
	{
		ptCol.y -= playerColumn.WingSize();
	}

	unsigned int count = 0;
	while (count < colLen)
	{
		GridContents targetColor = playerColumn.colors[colorPos % colLen];
		SetContents(ptCol, targetColor);
		++count;

		playerColumn.isHorizontal ? ++ptCol.x : ++ptCol.y;
		!playerColumn.isInverted ? ++colorPos : --colorPos;
	}
}

bool geng::columns::ColumnsSim::TransformPlayerColumn(const PlayerSet& target, const PointDelta& delta,
	                                                  bool checkValid)
{
	// First check if the transformation is possible
	if (checkValid && !IsValidShiftedPlayerColumn(target, delta))
	{
		return false;
	}

	// Remove what is there now
	RemovePlayerColumn(m_playerColumn);
	// Set the new column
	m_playerColumn = target;
	m_playerColumn.locCenter.x += delta.dx;
	m_playerColumn.locCenter.y += delta.dy;
	SetPlayerColumn(m_playerColumn);

	return true;
}

bool geng::columns::ColumnsSim::ShiftPlayerColumn( bool isLeft)
{
	// Shift the column 
	return TransformPlayerColumn(m_playerColumn, PointDelta{ isLeft ? -1 : 1, 0 }, true);
}

void geng::columns::ColumnsSim::ApplyRotation(PlayerSet& target, bool clockwise)
{
	// NOTE:  If a column has fewer than N different stones, the same apparent column
	// can be achieved both by permutation and rotation
	// This does not affect the logic in any way!

	// NOTE:  The reason for the different behavior of "inverted" in horizontal and vertical
	// configurations is the fact that the Y axis is anticartesian (increasing downward)
	// Therefore the meaning of "inverted" in horizontal and vertical orientations is juxtaposed

	//    R
	//  R   I
	//    I

	if (target.isHorizontal)
	{
		target.isHorizontal = false;
		target.isInverted = target.isInverted ? clockwise : !clockwise;
	}
	else
	{
		target.isHorizontal = true;
		target.isInverted = target.isInverted ? !clockwise : clockwise;
	}

}

bool geng::columns::ColumnsSim::CanPlayerColumnRotate(const PlayerSet& playerColumn, bool clockwise)
{
	PlayerSet newColumn{ playerColumn };
	ApplyRotation(newColumn, clockwise);
	return IsValidShiftedPlayerColumn(newColumn, PointDelta{ 0,0 });
}

bool geng::columns::ColumnsSim::RotatePlayerColumn(bool clockwise)
{
	PlayerSet newColumn{ m_playerColumn };
	ApplyRotation(newColumn, clockwise);
	return TransformPlayerColumn(newColumn, PointDelta{ 0,0 }, true);
}

bool geng::columns::ColumnsSim::ShouldLockPlayerColumn(const PlayerSet& playerColumn)
{
	return IsValidShiftedPlayerColumn(playerColumn, PointDelta{ 0, 1 });
}

bool geng::columns::ColumnsSim::DropPlayerColumn()
{
	return TransformPlayerColumn(m_playerColumn, PointDelta{ 0, 1 }, true);
}

bool geng::columns::ColumnsSim::PermutePlayerColumn()
{
	PlayerSet newColumn{ m_playerColumn };

	++newColumn.startPt;
	if (newColumn.startPt == newColumn.Width())
	{
		newColumn.startPt = 0;
	}

	// No need to check - shifts happen in-place
	TransformPlayerColumn(newColumn, PointDelta{ 0,0 }, false);
	return true;
}

void geng::columns::ColumnsSim::AddPlayerColumnToCompactSet(const PlayerSet& playerColumn)
{
	// This will succeed even if the column is vertical -- but will only create needless work
	// Also for vertical columns it will add the same X coordinate multiply
	unsigned int colLen = playerColumn.Width();
	Point locStart{ playerColumn.StartX(), playerColumn.StartY() };

	unsigned int count{ 0 };

	while (count < colLen)
	{
		m_columnsToCompact.emplace(locStart.x);

		++count;
		playerColumn.isHorizontal ? ++locStart.x : ++locStart.y;
	}
}


bool geng::columns::ColumnsSim::GenerateNewPlayerColumn()
{
	// First materialize the new player column
	PlayerSet newColumn;
	newColumn.locCenter.x = m_size.x / 2;
	newColumn.locCenter.y = (m_columnSize - 1) / 2;
	newColumn.colors = m_nextColors;

	// Can this column exist?
	if (!IsValidShiftedPlayerColumn(newColumn, PointDelta{ 0,0 }))
	{
		return false;
	}

	// isHorizontal, isInverted both false by default (see text of PlayerSet)
	// startPt is 0 by default
	m_playerColumn = std::move(newColumn);

	// Generate next colors
	m_nextColors.clear();

	for (size_t i = 0; i < m_columnSize; ++i)
	{
		GridContents nextColor = m_pInput->GetRandomNumber(1, GRID_LIMIT);
		m_nextColors.emplace_back(nextColor);
	}

	return true;
}

bool geng::columns::ColumnsSim::LockPlayerColumn()
{
	m_columnsToCompact.clear();
	if (m_playerColumn.isHorizontal)
	{
		AddPlayerColumnToCompactSet(m_playerColumn);
	}

	return GenerateNewPlayerColumn();
}

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
	// We assume the columns start with their bottom touching the first visible tiles
	Point topCorner{ 0, m_columnSize};
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

bool geng::columns::ColumnsSim::MarkRemovables(const std::vector<Point>& scanSet,
	size_t start,
	size_t end,
	unsigned int count)
{
	bool hasRemovables{ false };
	if (end - 1 - start >= count && IsRemovable(GetContents(scanSet[start])))
	{
		// Mark all points from "start" to "end" as removable
		for (size_t i = start; i < end; ++i)
		{
			const Point& toRemove = scanSet[i];
			m_toRemove.emplace(PointToIndex(toRemove));
			hasRemovables = true;
			m_columnsToCompact.emplace(toRemove.x);
		}
	}
	return hasRemovables;
}

bool geng::columns::ColumnsSim::ComputeRemovablesInSet(const std::vector<Point>& scanSet, unsigned int count)
{
	// Find count matching removable points in a row
	size_t seqStart = 0;
	size_t seqCur = 0;

	bool hasRemovables{ false };
	while (seqCur < scanSet.size())
	{
		if (GetContents(scanSet[seqStart]) != GetContents(scanSet[seqCur]))
		{
			if (MarkRemovables(scanSet, seqStart, seqCur, count))
			{
				hasRemovables = true;
			}
			seqStart = seqCur;
		}
		++seqCur;
	}

	// Last stretch
	if (MarkRemovables(scanSet, seqStart, seqCur, count))
	{
		hasRemovables = true;
	}

	return hasRemovables;
}

bool geng::columns::ColumnsSim::ComputeRemovables(unsigned int count)
{
	// Go through each stored set and scan it to find removables
	m_columnsToCompact.clear();
	m_toRemove.clear();

	bool hasRemovables{ false };
	for (size_t iSet = 0; iSet < m_setsToScan.size(); ++iSet)
	{
		if (ComputeRemovablesInSet(m_setsToScan[iSet], count))
		{
			hasRemovables = true;
		}
	}

	return hasRemovables;
}

void geng::columns::ColumnsSim::ExecuteRemove()
{
	
	auto removeGem = [](ColumnsSim& sim, unsigned int point,
		GridSquare& gc) -> bool
	{
		// Of course, "remove" just means "set to blank" so no structural changes
		sim.RemoveBlock(sim.IndexToPoint(point));
		return true;
	};

	IterateSet(m_toRemove.begin(), m_toRemove.end(),removeGem);
	m_toRemove.clear();
}

bool geng::columns::ColumnsSim::CompactColumn(unsigned int x)
{
	// This is just the partition algorithm applied to blank grid squares vs stones
	// in effect, the nonblanks all end up on the bottom

	// Returns true if at least one column was compacted
	bool compactedCol{ false };

	unsigned int readY = m_size.y - 1;
	unsigned int writeY = readY;
	while (readY > 0)
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
				compactedCol = true;
			}
			--readY;
			--writeY;
		}
	}

	return compactedCol;
}

bool geng::columns::ColumnsSim::CompactColumns()
{
	bool compactedCol{ false };
	for (unsigned int xCol : m_columnsToCompact)
	{
		if (CompactColumn(xCol))
		{
			compactedCol = true;
		}
	}
	m_columnsToCompact.clear();
	return compactedCol;
}

geng::columns::ColumnsSim::ColumnsSim(const ColumnsSimArgs& args)
	:BaseGameComponent("ColumnsSim", GameComponentType::Simulation),
	m_gameState(*this),
	m_dropAction("DropColumnAction", args.actionThrottlePeriod, *m_actionMapper),
	m_shiftLeftAction("ShiftColumnLeftAction", args.actionThrottlePeriod, *m_actionMapper),
	m_shiftRightAction("ShiftColumnRightAction", args.actionThrottlePeriod, *m_actionMapper),
	m_rotateAction("RotateColumnAction", args.actionThrottlePeriod, *m_actionMapper),
	m_permuteAction("PermuteColumnAction", args.actionThrottlePeriod, *m_actionMapper),
	m_size(args.boardSize),
	m_columnSize(args.columnSize),
	m_dropMiliseconds(args.dropMilliseconds),
	m_flashMiliseconds(args.flashMilliseconds),
	m_flashCount(args.flashCount),
	m_gameGrid(new GridSquare[args.boardSize.x * args.boardSize.y]),
	m_gameGridSize(args.boardSize.x * args.boardSize.y),
	m_inputName(args.pInputName),
	m_mapperName(args.pMapperName)
{
	GenerateNewPlayerColumn();	
}

bool geng::columns::ColumnsSim::Initialize(const std::shared_ptr<IGame>& pGame)
{
	GetComponentResult getResult;
	m_pInput = GetComponentAs<IInput>(pGame.get(), m_inputName.c_str(), getResult);

	if (!m_pInput)
	{
		pGame->LogError("ColumnsSim: could not get input component");
		return false;
	}

	m_actionMapper = GetComponentAs<ActionMapper>(pGame.get(), "ActionMapper", getResult);
	
	if (!m_pInput)
	{
		pGame->LogError("ColumnsSim: could not get action mapper");
		return false;
	}

	return true;
}

void geng::columns::ColumnsSim::OnFrame(IFrameManager* pManager)
{
	StateArgs stateArgs;
	
	SimState state;
	pManager->GetSimState(state, FID_SIMTIME);

	stateArgs.pFrameManager = pManager;
	stateArgs.simTime = state.simulatedTime;

	// Process inputs
	m_actionMapper->OnFrame(m_pInput.get());

	if (m_firstFrame)
	{
		m_gameState.Transition<DropColumnState>(m_gameState, stateArgs);
		m_firstFrame = false;
	}

	m_gameState.StartFrame();

	bool shouldContinue = true;
	while (shouldContinue)
	{
		shouldContinue = m_gameState.DispatchState(stateArgs);
	}

	m_gameState.EndFrame();
}


void geng::columns::ColumnsSim::GameState
::OnEnterState(DropColumnState& dropState, const StateArgs& args)
{
	dropState.nextDropTime = args.simTime + m_owner.m_dropMiliseconds;
}

void geng::columns::ColumnsSim::GameState
::OnState(DropColumnState& dropState, const StateArgs& stateArgs)
{
	// Drop state.  Exit conditions: 
	//    1. Drop->compact (no frame advance): The player column is "pushed" down but has nowhere to go;
	//       a new column is formed above the playing 
	//    2. Drop->game over (frame advance):  The player column is "pushed" down, but a new column
	//       could not be formed because the space to form it is taken.

	SetFrameComplete(true);

	// Drop?
	if (m_owner.m_dropAction.Triggered()
		|| dropState.nextDropTime <= stateArgs.simTime)
	{
		// Execute drop.  If a drop can't be executed, lock the player's gems and switch to Compact mode
		if (!m_owner.DropPlayerColumn())
		{
			// Lock the gems and generate a new column, then switch to compact mode
			if (!m_owner.LockPlayerColumn())
			{
				// Set to game over and terminate the frame
				Transition<GameOverState>(*this, stateArgs);
				return;
			}

			// Switch to compact mode and stay in frame
			Transition<CompactState>(*this, stateArgs);
			SetFrameComplete(false);
			return;
		}

		// Update the drop time
		dropState.nextDropTime = stateArgs.simTime + m_owner.m_dropMiliseconds;
	}

	// The other actions.
	if (m_owner.m_shiftLeftAction.Triggered())
	{
		if (!m_owner.ShiftPlayerColumn(true))
		{
			m_owner.m_errorText = "CAN'T MOVE";
			return;
		}
	}

	if (m_owner.m_shiftRightAction.Triggered())
	{
		if (!m_owner.ShiftPlayerColumn(false))
		{
			m_owner.m_errorText = "CAN'T MOVE";
			return;
		}
	}

	if (m_owner.m_rotateAction.Triggered())
	{
		if (!m_owner.RotatePlayerColumn(true))
		{
			m_owner.m_errorText  = "CAN'T ROTATE";
			return;
		}
	}

	if (m_owner.m_permuteAction.Triggered())
	{
		m_owner.PermutePlayerColumn();
		return;
	}
}

void geng::columns::ColumnsSim::GameState::
	OnState(CompactState& compactState, const StateArgs& stateArgs)
{
	// In compact state, we execute the compaction and immediately switch for next frame to
	// clear (same frame if nothing was compacted)
	bool hadCompacted = m_owner.CompactColumns();
	Transition<ClearState>(*this, stateArgs);

	SetFrameComplete(hadCompacted);
}


void geng::columns::ColumnsSim::GameState::
OnEnterState(ClearState& clearState, const StateArgs& stateArgs)
{
	// Compute the removables.   If the set is empty, immediately switch to 
	// drop column state.  Otherwise, we go to blinking

	// (hardcoding three)
	bool hasRemovables = m_owner.ComputeRemovables(3);

	if (hasRemovables)
	{
		// Initialize the blink phase
		clearState.blinkPhase = false;
		clearState.blinkPhaseCount = false;
		clearState.nextBlinkTime = stateArgs.simTime + m_owner.m_flashMiliseconds;

		// Implement
		SetBlinkState(m_owner.m_toRemove.begin(), m_owner.m_toRemove.end(), clearState.blinkPhase);
	}
	else
	{
		Transition<DropColumnState>(*this, stateArgs);
	}
}

void geng::columns::ColumnsSim::GameState::
OnState(ClearState& clearState, const StateArgs& stateArgs)
{
	SetFrameComplete(true);

	if (clearState.nextBlinkTime <= stateArgs.simTime)
	{
		++clearState.blinkPhaseCount;
		if (clearState.blinkPhaseCount == 2 * m_owner.m_flashCount)
		{
			// Done flashing.  Remove and go to compact again
			// When we return to this state after Compact, we will immediately go to Drop
			// if there's nothing to clear
			m_owner.ExecuteRemove();
			Transition<CompactState>(*this, stateArgs);
			return;
		}
		clearState.blinkPhase = !clearState.blinkPhase;
		clearState.nextBlinkTime = stateArgs.simTime + m_owner.m_flashMiliseconds;
		SetBlinkState(m_owner.m_toRemove.begin(), m_owner.m_toRemove.end(), clearState.blinkPhase);
	}
}