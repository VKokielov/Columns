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

	// thanks JJ
	if (idx < m_gameGridSize)
	{
		if (pisvalid)
		{
			*pisvalid = true;
		}
		return m_gameGrid[idx].contents;
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
	if ((int)target.CenterX() < -delta.dx 
	|| target.CenterX() + delta.dx >= m_size.x
	|| (int)target.CenterY() < -delta.dy
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
		bool inPlayerColumn = m_validPlayerColumn && m_playerColumn.InColumn(newStart);

		if (!inPlayerColumn
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

	if (playerColumn.isInverted)
	{
		--colorPos;
	}

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
//	fprintf(stderr, "Set column\n");
	while (count < colLen)
	{
	//	fprintf(stderr, "%lu/%lu ", (unsigned long)colorPos, (unsigned long)(colorPos % colLen));

		GridContents targetColor = playerColumn.colors[colorPos % colLen];
		SetContents(ptCol, targetColor);
		++count;

		playerColumn.isHorizontal ? ++ptCol.x : ++ptCol.y;
		!playerColumn.isInverted ? ++colorPos : --colorPos;
	}
	//fprintf(stderr, "/Set column\n");

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

void geng::columns::ColumnsSim::GenerateNextColors()
{
	m_nextColors.clear();

	for (size_t i = 0; i < m_columnSize; ++i)
	{
		GridContents nextColor = m_pInput->GetRandomNumber(1, GRID_LIMIT-1);
		m_nextColors.emplace_back(nextColor);
	}
}

bool geng::columns::ColumnsSim::GenerateNewPlayerColumn()
{
	// First materialize the new player column
	if (m_nextColors.empty())
	{
		GenerateNextColors();
	}

	PlayerSet newColumn;
	newColumn.locCenter.x = m_size.x / 2;
	newColumn.locCenter.y = (m_columnSize - 1) / 2;
	newColumn.colors = m_nextColors;

	// Invalidate the player column -- so game over happens right
	m_validPlayerColumn = false;

	// Can this column exist?
	if (!IsValidShiftedPlayerColumn(newColumn, PointDelta{ 0,0 }))
	{
		return false;
	}

	// isHorizontal, isInverted both false by default (see text of PlayerSet)
	// startPt is 0 by default
	m_playerColumn = std::move(newColumn);
	TransformPlayerColumn(m_playerColumn, PointDelta{ 0,0 }, false);
	
	m_validPlayerColumn = true;

	// Generate next colors
	GenerateNextColors();

	return true;
}

bool geng::columns::ColumnsSim::LockPlayerColumn()
{
	m_columnsToCompact.clear();
	if (m_playerColumn.isHorizontal)
	{
		AddPlayerColumnToCompactSet(m_playerColumn);
	}

	/*
	fprintf(stderr, "Locking player column at center %d %d\n", m_playerColumn.locCenter.x, m_playerColumn.locCenter.y);
	if (m_playerColumn.locCenter.y == 1)
	{
		fprintf(stderr, "\t%d %d %d\n", GetContents(Point{ 4, 0 }), GetContents(Point{ 4,1 }), GetContents(Point{ 4,2 }));
	}
	m_validPlayerColumn = false;
	*/

	return GenerateNewPlayerColumn();
}

geng::columns::GridSquare* 
	geng::columns::ColumnsSim::GetContentsOfPredecessor(const Point& at, unsigned int seqIdx, bool isTopDown,
		Point& ptNext)
{
	// Compute the next point
	ptNext = at;

	switch (seqIdx)
	{
	case SEQ_NE:
		isTopDown ? --ptNext.x : ++ptNext.x;
		isTopDown ? --ptNext.y : ++ptNext.y;
		break;
	case SEQ_N:
		isTopDown ? --ptNext.y : ++ptNext.y;
		break;
	case SEQ_NW:
		isTopDown ? ++ptNext.x : --ptNext.y;
		isTopDown ? --ptNext.y : ++ptNext.y;
		break;
	case SEQ_E:
		isTopDown ? --ptNext.x : ++ptNext.x;
		break;
	}

	if (!InBounds(ptNext))
	{
		return nullptr;
	}

	return &m_gameGrid[PointToIndex(ptNext)];
}

bool geng::columns::ColumnsSim::ComputeRemovables(unsigned int count)
{
	// Go through each stored set and scan it to find removables
	m_columnsToCompact.clear();
	m_toRemove.clear();

	for (unsigned int idx = 0; idx < m_gameGridSize; ++idx)
	{
		Point curPt = IndexToPoint(idx);

		GridSquare& curSquare = m_gameGrid[idx];
		curSquare.wasRemoved = false;

		if (!IsRemovable(curSquare.contents))
		{
			// Ignore squares that cannot be removed
			// The sequence numbers are of no interest as we are comparing for equality
			continue;
		}

		for (unsigned int seqIdx = 0; seqIdx < 4; ++seqIdx)
		{
			Point predPt;
			GridSquare* pPredecessor = GetContentsOfPredecessor(curPt, seqIdx, true, predPt);

			if (pPredecessor && pPredecessor->contents == curSquare.contents)
			{
//				fprintf(stderr, "ex %d %d\n", curPt.x, curPt.y);
//				fprintf(stderr, "Found predecessor\n");
				curSquare.seqNumbers[seqIdx] = pPredecessor->seqNumbers[seqIdx] + 1;

				// Mark me and up to n-1 of my predecessors
				if (curSquare.seqNumbers[seqIdx] >= count)
				{
					curSquare.wasRemoved = true;
					m_columnsToCompact.emplace(curPt.x);
					m_toRemove.emplace_back(idx);

					GridSquare* pSetRem = pPredecessor;
					int nToSet = count - 1;
					while (nToSet > 0)
					{
						if (!pSetRem->wasRemoved)
						{
							pSetRem->wasRemoved = true;
							
							m_columnsToCompact.emplace(predPt.x);
							m_toRemove.emplace_back(PointToIndex(predPt));
						}

						// Try to get the next predecessor
						Point nextPred;
						pSetRem = GetContentsOfPredecessor(predPt, seqIdx, true, nextPred);

						if (!pSetRem)
						{
							break;
						}

						predPt = nextPred;
						--nToSet;
					}
				}
			}
			else
			{
				curSquare.seqNumbers[seqIdx] = 1;
			}
		}
	}

	return !m_toRemove.empty();
}

void geng::columns::ColumnsSim::ExecuteRemove()
{
	
	auto removeGem = [this](ColumnsSim& sim, unsigned int point,
		GridSquare& gc) -> bool
	{
		// Of course, "remove" just means "set to blank" so no structural changes
		RemoveBlock(IndexToPoint(point));
		
		return true;
	};

	IterateSet(m_toRemove.begin(), m_toRemove.end(),removeGem);
	m_clearedGems += (unsigned int)m_toRemove.size();
	m_clearedGemsInLevel += (unsigned int)m_toRemove.size();
	m_toRemove.clear();
}

bool geng::columns::ColumnsSim::ShouldLevelUp()
{
	return m_clearedGemsInLevel >= m_levelThreshhold;
}
void geng::columns::ColumnsSim::LevelUp()
{
	// Update times, etc
	// Speed up by 10%
	m_curDropMiliseconds -= m_curDropMiliseconds / 10;
	if (m_curDropMiliseconds == m_minDropMiliseconds)
	{
		m_curDropMiliseconds = m_minDropMiliseconds;
	}

	m_clearedGemsInLevel = 0;
	m_levelThreshhold += (m_levelThreshhold / 10);
	m_levelThreshhold -= m_levelThreshhold % m_columnSize;

	++m_level;
}

bool geng::columns::ColumnsSim::CompactColumn(unsigned int x)
{
	// This is just the partition algorithm applied to blank grid squares vs stones
	// in effect, the nonblanks all end up on the bottom

	// Returns true if at least one column was compacted
	bool compactedCol{ false };

	unsigned int readY = m_size.y - 1;
	unsigned int writeY = readY;

	unsigned int topLimit = 0;

	// To avoid "compacting" player gems that have been generated,
	// specialize this 
	if (x == m_size.x / 2)
	{
		topLimit = m_columnSize-1;
	}

	while (readY > topLimit)
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

const char* geng::columns::ColumnsSim::GetDropActionName()
{
	return "DropColumnAction";
}
const char* geng::columns::ColumnsSim::GetShiftLeftActionName()
{
	return "ShiftColumnLeftAction";
}
const char* geng::columns::ColumnsSim::GetShiftRightActionName()
{
	return "ShiftColumnRightAction";
}
const char* geng::columns::ColumnsSim::GetRotateActionName()
{
	return "RotateColumnAction";
}
const char* geng::columns::ColumnsSim::GetPermuteActionName()
{
	return "PermuteColumnAction";
}

geng::columns::ColumnsSim::SimActionWrappers::SimActionWrappers(unsigned int throttlePeriod, 
							unsigned int dropThrottlePeriod,
							ActionMapper& mapper)
	:dropAction(GetDropActionName(), dropThrottlePeriod, mapper)
	,shiftLeftAction(GetShiftLeftActionName(), throttlePeriod, mapper)
	,shiftRightAction(GetShiftRightActionName(), throttlePeriod, mapper)
	,rotateAction(GetRotateActionName(), throttlePeriod, mapper)
	,permuteAction(GetPermuteActionName(), throttlePeriod, mapper)
{
}

void geng::columns::ColumnsSim::SimActionWrappers::UpdateState(ActionMapper& mapper, unsigned long simTime)
{
	dropAction.UpdateState(mapper, simTime);
	shiftLeftAction.UpdateState(mapper, simTime);
	shiftRightAction.UpdateState(mapper, simTime);
	rotateAction.UpdateState(mapper, simTime);
	permuteAction.UpdateState(mapper, simTime);
}

geng::columns::ColumnsSim::ColumnsSim(const ColumnsSimArgs& args)
	:BaseGameComponent("ColumnsSim", GameComponentType::Simulation),
	m_gameState(*this),
	m_size(args.boardSize),
	m_columnSize(args.columnSize),
	m_dropMiliseconds(args.dropMilliseconds),
	m_flashMiliseconds(args.flashMilliseconds),
	m_flashCount(args.flashCount),
	m_gameGrid(new GridSquare[args.boardSize.x * args.boardSize.y]),
	m_gameGridSize(args.boardSize.x * args.boardSize.y),
	m_inputName(args.pInputName),
	m_throttlePeriod(args.actionThrottlePeriod),
	m_dropThrottlePeriod(args.dropThrottlePeriod)
{
	// Clear the grid
	GridSquare defaultSquare{ EMPTY, true};
	std::fill(m_gameGrid.get(), m_gameGrid.get() + m_gameGridSize, defaultSquare);

}

geng::IFrameListener* geng::columns::ColumnsSim::GetFrameListener()
{
	return this;
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
	
	if (!m_actionMapper)
	{
		pGame->LogError("ColumnsSim: could not get action mapper");
		return false;
	}

	m_actionWrappers = std::make_shared<SimActionWrappers>(m_throttlePeriod,
														m_dropThrottlePeriod,
														*m_actionMapper.get());

	// Save the parameter
	m_curDropMiliseconds = m_dropMiliseconds;
	// 10 times faster is good enough
	m_minDropMiliseconds = m_dropMiliseconds / 10;

	GenerateNewPlayerColumn();

	return true;
}

void geng::columns::ColumnsSim::OnFrame(IFrameManager* pManager)
{
	StateArgs stateArgs;
	
	SimState state;
	pManager->GetSimState(state, FID_SIMTIME);

	stateArgs.pFrameManager = pManager;
	stateArgs.simTime = state.simulatedTime;

	if (m_firstFrame)
	{
		m_gameState.Transition<DropColumnState>(m_gameState, stateArgs);
		m_firstFrame = false;
	}

	// This will update the actions with the state of the keyboard
	//fprintf(stderr, "updst\n");
	m_actionWrappers->UpdateState(*m_actionMapper, state.simulatedTime);

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
	dropState.nextDropTime = args.simTime + m_owner.m_curDropMiliseconds;
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
	if (m_owner.m_actionWrappers->dropAction.Triggered()
		|| dropState.nextDropTime <= stateArgs.simTime)
	{
		// Execute drop.  If a drop can't be executed, lock the player's gems and switch to Compact mode
		if (!m_owner.DropPlayerColumn())
		{
			// Lock the gems and generate a new column, then switch to compact mode
			if (!m_owner.LockPlayerColumn())
			{
				// Set to game over and terminate the frame
				fprintf(stderr, "game over\n");
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
	if (m_owner.m_actionWrappers->shiftLeftAction.Triggered())
	{
		if (!m_owner.ShiftPlayerColumn(true))
		{
			m_owner.m_errorText = "CAN'T MOVE";
			return;
		}
	//	fprintf(stderr, "Shifting column left\n");
	}

	if (m_owner.m_actionWrappers->shiftRightAction.Triggered())
	{
		if (!m_owner.ShiftPlayerColumn(false))
		{
			m_owner.m_errorText = "CAN'T MOVE";
			return;
		}
	//	fprintf(stderr, "Shifting column right\n");
	}

	if (m_owner.m_actionWrappers->rotateAction.Triggered())
	{
		if (!m_owner.RotatePlayerColumn(true))
		{
			m_owner.m_errorText  = "CAN'T ROTATE";
			return;
		}
	}

	if (m_owner.m_actionWrappers->permuteAction.Triggered())
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
	bool hasRemovables = m_owner.ComputeRemovables(m_owner.m_columnSize);

	if (hasRemovables)
	{
		// Initialize the blink phase
		clearState.blinkPhase = true;
		clearState.blinkPhaseCount = 0;
		clearState.nextBlinkTime = 0.5 * (stateArgs.simTime + m_owner.m_flashMiliseconds);

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
		clearState.blinkPhase = !clearState.blinkPhase;
		SetBlinkState(m_owner.m_toRemove.begin(), m_owner.m_toRemove.end(), clearState.blinkPhase);

		if (clearState.blinkPhaseCount == 2 * m_owner.m_flashCount)
		{
			// Done flashing.  Remove and go to compact again
			// When we return to this state after Compact, we will immediately go to Drop
			// if there's nothing to clear
			m_owner.ExecuteRemove();
			// Level up?
			if (m_owner.ShouldLevelUp())
			{
				m_owner.LevelUp();
			}

			Transition<CompactState>(*this, stateArgs);
			return;
		}
		clearState.nextBlinkTime = stateArgs.simTime + m_owner.m_flashMiliseconds;
	}
}