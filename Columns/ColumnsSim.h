#pragma once

#include "IInput.h"
#include "ActionMapper.h"

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

	struct GridSquare
	{
		GridContents  contents;
		bool isVisible;
	};

	struct Point
	{
		unsigned int x;
		unsigned int y;
	};

	enum class ColumnDir
	{
		Upright,
		LeftToRight,
		UpsideDown,
		RightToLeft
	};

	struct PlayerSet
	{
		Point locCenter;
		std::vector<GridContents> colors;
		ColumnDir columnDir{ ColumnDir::Upright };
		// Shift
		unsigned int startPt{ 0 };

		// Wing size
		unsigned int WingSize() const
		{
			return (colors.size() - 1) / 2;
		}
	};

	class ColumnsSim
	{
	private:

	public:

	private:

		// Will the player column need to be locked once it moves down?
		bool ShouldLockColumn();
		// Is there room for the player column to rotate?
		bool CanPlayerColumnBeOriented(const PlayerSet& playerColumn, bool isH);
		// Can the player column shift?
		bool CanPlayerColumnShift(const PlayerSet& playerColumn, bool isLeft);

		// Add or remove the stones in a column to the board
		void RemovePlayerColumn(const PlayerSet& playerColumn);
		void SetPlayerColumn(const PlayerSet& playerColumn);

		void RotatePlayerColumn(PlayerSet& playerColumn, ColumnDir targetDir);
		// Fill the compact set with the locations of the player column (if compaction is needed)
		// NOTE:  Used when the player column is locked in horizontal state
		void AddPlayerColumnToCompactSet(const PlayerSet& playerColumn);

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

		// Moves a block by overwriting.  Returns "false" if "to" was not blank
		bool MoveBlock(const Point& from, const Point& to);
		bool RemoveBlock(const Point& at);
		GridContents GetContents(const Point& at) const;

		// Using indices makes it easier 
		unsigned int PointToIndex(const Point& at) const;
		Point IndexToPoint(unsigned int idx) const;

		// Actions
		ActionMapper  m_actionMapper;
		ActionID m_shiftAction;
		ActionID m_turnAction;
		ActionID m_permuteAction;
		ActionID m_speedUpAction;

		// __Parameters__
		Point m_size;
		unsigned int m_overflow;  // Vertical overflow - should be column size - 1

		// __Removable sets__
		// These are sets of points to scan for sequences of identical entries
		// They are parallel lines in different directions (horizontal, vertical, downsloping diagonal, upsloping diagonal)
		// We save them in order to be able to scan quickly
		std::vector<std::vector<Point> >  m_setsToScan;

		// _Simulation state_
		std::unique_ptr<GridSquare[]>  m_gameGrid;
		std::unordered_set<unsigned int> m_toRemove;
		// Store x coordinates of points to remove because that is where the holes will appear
		// (This is an optimization to prevent needless work)
		std::unordered_set<unsigned int> m_columnsToCompact;  
		
		PlayerSet  m_playerColumn;
		PlayerSet  m_nextColumn;



	};
}