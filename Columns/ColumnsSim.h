#pragma once

#include "IInput.h"
#include "ActionMapper.h"
#include "SimStateDispatcher.h"
#include "BaseGameComponent.h"

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

	constexpr unsigned int SEQ_NE = 0;
	constexpr unsigned int SEQ_N = 1;
	constexpr unsigned int SEQ_NW = 2;
	constexpr unsigned int SEQ_E = 3;

	struct GridSquare
	{
		GridContents  contents;
		bool isVisible;
		
		bool wasRemoved;
		std::array<unsigned int, 4>   seqNumbers;
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
			return (unsigned int)colors.size();
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
		unsigned int columnSize;
		unsigned int dropMilliseconds;
		unsigned int flashMilliseconds;
		unsigned int flashCount;
		unsigned int actionThrottlePeriod;
		unsigned int dropThrottlePeriod;

		// Component names
		const char* pInputName;
	};

	class ColumnsSim : public IGameListener, 
						public BaseGameComponent
	{
	private:
		struct InitialState { };
		struct DropColumnState 
		{ 
			unsigned long nextDropTime{ 0 };
		};
		struct CompactState { };
		struct ClearState 
		{ 
			bool blinkPhase{ false };
			unsigned long nextBlinkTime{ 0 };
			unsigned int blinkPhaseCount{ 0 };
		};

		struct GameOverState { };

		struct StateArgs
		{
			unsigned long simTime;
		};

		struct SimActionWrappers
		{
			ThrottledActionWrapper dropAction;
			ThrottledActionWrapper shiftLeftAction;
			ThrottledActionWrapper shiftRightAction;
			ThrottledActionWrapper rotateAction;
			ThrottledActionWrapper permuteAction;

			SimActionWrappers(unsigned int throttlePeriod, 
				unsigned int dropThrottlePeriod, 
				ActionMapper& mapper);

			void UpdateState(ActionMapper& mapper, unsigned long simTime);
		};

		// Drop->(lock)->Compact->Clear->?Compact,Drop
		class GameState : public SimStateDispatcher<GameState,
													InitialState,
													InitialState,
													DropColumnState, 
												    CompactState, 
													ClearState,
													GameOverState>
		{
		public:


			GameState(ColumnsSim& owner) 
				:m_owner(owner)
			{ }

			void StartFrame()
			{
				m_frameComplete = false;
			}

			void EndFrame() { }

			// return 
			bool DispatchState(const StateArgs& stateArgs)
			{
				Dispatch(*this, stateArgs);

				return !m_frameComplete;
			}

			bool IsFrameComplete() const { return m_frameComplete; }

			// Generic state callbacks for any unimplemented states
			template<typename T>
			void OnEnterState(T& state, 
				const StateArgs&) { }

			template<typename T>
			void OnState(T& state, const StateArgs&)
			{ 
				// By default just skip the frame
				SetFrameComplete(true);
			}

			template<typename T>
			void OnExitState(T& state, const StateArgs&) { }

			void OnEnterState(DropColumnState& state, 
				const StateArgs& args);

			void OnEnterState(ClearState& state, const StateArgs& args);

			void OnState(DropColumnState& dropState, const StateArgs& stateArgs);
			void OnState(CompactState& compactState, const StateArgs& stateArgs);
			void OnState(ClearState& clearState, const StateArgs& stateArgs);

		private:
			void SetFrameComplete(bool val)
			{
				m_frameComplete = val;
			}

			template<typename I>
			void SetBlinkState(I bSet, I eSet, bool state)
			{
				auto setBlink = [state](ColumnsSim& sim, unsigned int point,
					GridSquare& gc)
				{
					gc.isVisible = state;
					return true;
				};

				m_owner.IterateSet(bSet, eSet, setBlink);
			}

			ColumnsSim&   m_owner;
			// As long as this is false, the frame callback loops on the state
			// This is analogous to revisiting the same character in different states
			// during a parse
			bool m_frameComplete{ false };
		};

	public:
		ColumnsSim(const ColumnsSimArgs& args);
		bool Initialize(const std::shared_ptr<IGame>& pGame) override;

		void OnFrame(const SimState& rSimState,
			const SimContextState* pContextState) override;

		static const char* GetDropActionName();
		static const char* GetShiftLeftActionName();
		static const char* GetShiftRightActionName();
		static const char* GetRotateActionName();
		static const char* GetPermuteActionName();

		unsigned int PointToIndex(const Point& at) const;
		Point IndexToPoint(unsigned int idx) const;
		// This gets the "predecessor" tile for the algorithm that computes what should be removed
		GridSquare* GetContentsOfPredecessor(const Point& at, unsigned int seqIdx, bool isDown,
			                                  Point& predPt);

		Point GetBoardSize() const { return m_size; }
		unsigned int GetColumnSize() const { return m_columnSize; }

		template<typename F>
		void IterateGrid(F&& callback, unsigned int firstIndex = 0) const
		{
			for (unsigned long idx = firstIndex; idx < m_gameGridSize; ++idx)
			{
				Point curPt = IndexToPoint(idx);
				callback(curPt, m_gameGrid[idx]);
			}
		}

		const std::vector<GridContents>& GetNextColors() const 
		{
			return m_nextColors; 
		}

		const unsigned int GetLevel() const { return m_level; }
		const unsigned int GetGems() const { return m_clearedGems; }
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

		template<typename I, typename F>
		void IterateSet(I bSet, I eSet, F&& callback)
		{
			while (bSet != eSet)
			{
				unsigned int point = *bSet;
				GridSquare& gsquare = m_gameGrid[point];
				if (!callback(*this, point, gsquare))
				{
					return;
				}
				++bSet;
			}
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
		bool DropPlayerColumn();

		// Pro-forma -- permutation always works
		bool PermutePlayerColumn();

		// Fill the compact set with the locations of the player column (if compaction is needed)
		// NOTE:  Used when the player column is locked in horizontal state
		void AddPlayerColumnToCompactSet(const PlayerSet& playerColumn);
		void GenerateNextColors();
		bool GenerateNewPlayerColumn();
		bool LockPlayerColumn();

		// __Removables__

		bool ComputeRemovables(unsigned int count);
		void ExecuteRemove();
		bool ShouldLevelUp();
		void LevelUp();

		// Compact columns by shifting all stones down
		bool CompactColumn(unsigned int x);
		bool CompactColumns();

		// Actions
		std::shared_ptr<ActionMapper>  m_actionMapper;
		std::shared_ptr<SimActionWrappers>  m_actionWrappers;

		// __Parameters__
		Point m_size;
		unsigned int m_columnSize;
		unsigned int m_dropMiliseconds;
		unsigned int m_flashMiliseconds;
		unsigned int m_flashCount;
		unsigned int m_throttlePeriod;
		unsigned int m_dropThrottlePeriod;
		std::string m_inputName;

		std::shared_ptr<IInput>  m_pInput;

		// __Removable sets__

		// _Simulation state_

		std::vector<unsigned int> m_toRemove;
		// Store x coordinates of points to remove because that is where the holes will appear
		// (This is an optimization to prevent needless work)
		std::unordered_set<unsigned int> m_columnsToCompact;  
		
		bool m_validPlayerColumn{ false };
		PlayerSet  m_playerColumn;
		std::vector<GridContents>  m_nextColors;

		std::string m_errorText;

		std::unique_ptr<GridSquare[]>  m_gameGrid;
		size_t m_gameGridSize;

		GameState m_gameState;

		bool m_firstFrame{ true };

		unsigned int m_clearedGems{ 0 };
		unsigned int m_clearedGemsInLevel{ 0 };
		unsigned int m_levelThreshhold{ 45 };
		unsigned int m_level{ 1 };

		unsigned int m_curDropMiliseconds{ 0 };
		unsigned int m_minDropMiliseconds{ 0 };

	};
}