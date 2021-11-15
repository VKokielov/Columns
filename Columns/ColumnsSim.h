#pragma once

#include "IInput.h"
#include "ActionMapper.h"
#include "SimStateDispatcher.h"

#include <memory>
#include <array>
#include <unordered_set>

namespace geng::columns
{

	using GridContents = int;

	constexpr GridContents EMPTY = 0;
	constexpr GridContents RED = 1;
	constexpr GridContents GREEN = 2;
	constexpr GridContents YELLOW = 3;
	constexpr GridContents MAGENTA = 4;
	constexpr GridContents BLUE = 5;
	constexpr GridContents GRID_LIMIT = 6;

	struct GridSquare
	{
		GridContents  contents;
		bool isVisible;
	};

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
	
	struct PointDelta
	{
		int dx;
		int dy;
	};

	struct PlayerSet
	{
		Point locCenter;
		std::vector<GridContents> colors;
		bool isHorizontal{ false };
		bool isInverted{ false };
		// Shift
		unsigned int startPt{ 0 };

		unsigned int Width() const
		{
			return colors.size();
		}
		// Wing size
		unsigned int WingSize() const
		{
			return (Width() - 1) / 2;
		}

		unsigned int CenterX() const
		{
			return locCenter.x;
		}

		unsigned int StartX() const
		{
			return isHorizontal ? locCenter.x - WingSize() : locCenter.x;
		}

		unsigned int StartY() const
		{
			return isHorizontal ? locCenter.y - WingSize() : locCenter.y;
		}

		unsigned int CenterY() const
		{
			return locCenter.y;
		}

		bool InColumn(const Point& pt) const
		{
			unsigned int wingSize = WingSize();

			if (isHorizontal)
			{
				return locCenter.y == pt.y
					&& locCenter.x - wingSize <= pt.x
					&& pt.x <= locCenter.x + wingSize;
			}
			else
			{
				return locCenter.x == pt.x
					&& locCenter.y - wingSize <= pt.y
					&& pt.y <= locCenter.y + wingSize;
			}

			return false;
		}
	};

	struct ColumnsSimArgs
	{
		Point boardSize;
		unsigned long colSize;  // must be odd
		std::shared_ptr<ActionMapper>  pActionMapper;
	};

	class ColumnsSim
	{
	private:

		struct DropColumnState { };
		struct CompactState { };
		struct ClearState 
		{ };
		
		class GameState : public SimStateDispatcher<DropColumnState, CompactState, ClearState>
		{

		};

	public:
		ColumnsSim(const ColumnsSimArgs& args);

	private:
		// Starting at grid location X, check whether there are enough blocks of the same color to remove along
		// an axis (horizontal, vertical, downslope, upslop)
		enum class Axis
		{
			Horizontal,
			Vertical,
			Downslope,
			Upslope
		};

		bool InBounds(const Point& at) const
		{
			return at.x < m_size.x && at.y < m_size.y;
		}

		template<typename F>
		void IterateAxis(const Point& start, Axis axis, F&& callback)
		{
			// Iterate through all points on an axis
			Point pt{ start };

			while (InBounds(pt))
			{
				if (!callback(pt))
				{
					break;
				}
				switch (axis)
				{
				case Axis::Horizontal:
					++pt.x;
					break;
				case Axis::Vertical:
					++pt.y;
					break;
				case Axis::Downslope:
					++pt.x;
					++pt.y;
					break;
				case Axis::Upslope:
					--pt.x;
					++pt.y;
					break;
				}
			}
		}

		static bool IsRemovable(GridContents contents)
		{
			// Todo: generalize?
			return contents == RED
				|| contents == GREEN
				|| contents == YELLOW
				|| contents == MAGENTA
				|| contents == BLUE;
		}

		static bool IsBlank(GridContents contents)
		{
			return contents == EMPTY;
		}

		// _Grid operations_ 
		unsigned int PointToIndex(const Point& at) const;
		Point IndexToPoint(unsigned int idx) const;

		GridContents GetContents(const Point& at, bool* isValid = nullptr) const;
		bool SetContents(const Point& at, GridContents contents);
		// Moves a block by overwriting.  Returns "false" if "to" was not blank
		bool MoveBlock(const Point& from, const Point& to);
		bool RemoveBlock(const Point& at);


		// Add or remove the stones in a column to the board
		// Assumes all inputs are valid (enforced by the "Can" functions above)
		void RemovePlayerColumn(const PlayerSet& playerColumn);
		void SetPlayerColumn(const PlayerSet& playerColumn);

		// Check whether a column of a type with a given center delta is valid
		// We use deltas because points are pairs of unsigned coordinates
		bool IsValidShiftedPlayerColumn(const PlayerSet& target, const PointDelta& delta) const;
		bool TransformPlayerColumn(const PlayerSet& target, const PointDelta& delta, bool checkValid);

		// Can the player column shift?
		bool CanPlayerColumnShift(const PlayerSet& playerColumn, bool isLeft);
		bool ShiftPlayerColumn(bool isLeft);

		void ApplyRotation(PlayerSet& set, bool clockwise);
		bool CanPlayerColumnRotate(const PlayerSet& playerColumn, bool clockwise);
		bool RotatePlayerColumn(bool clockwise);

		// Can the player column move down or should it be locked?
		bool ShouldLockPlayerColumn(const PlayerSet& playerColumn);

		// Pro-forma -- permutation always works
		bool PermutePlayerColumn();

		// Fill the compact set with the locations of the player column (if compaction is needed)
		// NOTE:  Used when the player column is locked in horizontal state
		void AddPlayerColumnToCompactSet(const PlayerSet& playerColumn);

		void GenerateNewPlayerColumn(IInput* pRandomizer);

		// __Grid sets__ (for computing removables)
		void AddSetFromPoint(bool isEnabled, Axis axis, 
			const Point& origin,
			unsigned long minSize);
		void GenerateGridSets();

		// __Removables__
		void MarkRemovables(const std::vector<Point>& scanSet, size_t start, size_t end, unsigned int count);
		void ComputeRemovablesInSet(const std::vector<Point>& scanSet, unsigned int count);
		void ComputeRemovables(unsigned int count);

		// Compact columns by shifting all stones down
		void CompactColumn(unsigned int x);
		void CompactColumns();

		// Actions
		ActionMapper  m_actionMapper;
		ActionID m_shiftAction;
		ActionID m_turnAction;
		ActionID m_permuteAction;
		ActionID m_speedUpAction;

		// __Parameters__
		Point m_size;
		unsigned int m_columnSize;

		// __Removable sets__
		// These are sets of points to scan for sequences of identical entries
		// They are parallel lines in different directions (horizontal, vertical, downsloping diagonal, upsloping diagonal)
		// We save them in order to be able to scan quickly
		std::vector<std::vector<Point> >  m_setsToScan;

		// _Simulation state_

		std::unordered_set<unsigned int> m_toRemove;
		// Store x coordinates of points to remove because that is where the holes will appear
		// (This is an optimization to prevent needless work)
		std::unordered_set<unsigned int> m_columnsToCompact;  
		
		PlayerSet  m_playerColumn;
		std::vector<GridContents>  m_nextColors;

		std::unique_ptr<GridSquare[]>  m_gameGrid;
		size_t m_gameGridSize;
	};
}