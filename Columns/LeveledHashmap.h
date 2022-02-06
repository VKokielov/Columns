#pragma once

#include <type_traits>
#include <unordered_map>
#include <iterator>

namespace geng::lib
{

	// A wrapper for a hash map with keys consisting of several levels 
	// Executed in the simplest possible way.  

	template<typename ValueType,
		typename FirstKey,
		typename ... RestKeys>
	class LeveledHashmap;

	template<typename ValueType,
		     typename ... OtherTypes>
	struct LevelElementType
	{
		using type = LeveledHashmap<ValueType, OtherTypes...>;
	};

	template<typename ValueType>
	struct LevelElementType<ValueType>
	{
		using type = ValueType;
	};

	template<typename ValueType, typename ...OtherTypes>
	using LevelElementType_t =
		typename LevelElementType<ValueType, OtherTypes...>::type;

	template<typename ValueType,
		typename FirstKey,
		typename ... RestKeys>
	class LeveledHashmap
	{
	public:
		static constexpr bool isLeaf = sizeof...(RestKeys) == 0;
		using TElement = LevelElementType_t<ValueType, RestKeys...>;

		using TLevelKey = FirstKey;
		using TValue = ValueType;
		using TIter = typename std::unordered_map<FirstKey, TElement>::iterator;

		LeveledHashmap() = default;

		LeveledHashmap(std::size_t depth)
			:m_depth(depth)
		{ }

		// Insert, remove, find, iterate

		// Partial insert
		auto Insert(const FirstKey& key)
		{
			if constexpr (isLeaf)
			{
				return m_childMap.emplace(key, TElement());
			}
			else
			{
				return m_childMap.emplace(key, TElement(m_depth + 1));
			}
		}

		// Erase
		auto Erase(const FirstKey& key)
		{
			return m_childMap.erase(key);
		}

		auto Erase(const TIter& iter)
		{
			return m_childMap.erase(iter);
		}

		// Find
		auto Find(const FirstKey& key)
		{
			return m_childMap.find(key);
		}

		auto Find(const FirstKey& key) const
		{
			return m_childMap.find(key);
		}

		// Iterate
		auto Begin()
		{
			return std::begin(m_childMap);
		}

		auto Begin() const
		{
			return std::cbegin(m_childMap);
		}

		auto CBegin() const
		{
			return std::cbegin(m_childMap);
		}

		auto End()
		{
			return std::end(m_childMap);
		}

		auto End() const
		{
			return std::cend(m_childMap);
		}

		auto CEnd() const
		{
			return std::cend(m_childMap);
		}

		std::size_t GetDepth() const { return m_depth; }
		bool IsEmpty() const { return m_childMap.empty(); }

	private:

		std::unordered_map<FirstKey, TElement> m_childMap;
		std::size_t m_depth{ 0 };
	};

	// Helper functions for operations on data

	// Find
	// It's not possible to return an iterator here, since "begin" and "end"
	// are ill-defined when there are many maps
	namespace map_helpers
	{
		template<typename Lvl, typename K, size_t depth = 0>
		std::conditional_t<std::is_const_v<Lvl>,
			const typename Lvl::TValue*,
			typename Lvl::TValue*> Find(Lvl& lvl, const K& key)
		{
			auto&& levelKey = K::Getter<depth>::Get(key);


			if constexpr (Lvl::isLeaf)
			{
				auto levelIter = lvl.Find(levelKey);
				if (levelIter != lvl.End())
				{
					return &levelIter->second;
				}
			}
			else
			{
				// Using-type declaration for next level
				using TNextLevel
					= std::conditional_t < std::is_const_v<Lvl>,
					const typename Lvl::TElement,
					typename Lvl::TElement>;

				// "Linear recursion"
				auto itNextLevel = lvl.Find(levelKey);
				if (itNextLevel != lvl.End())
				{
					return Find<TNextLevel, K, depth + 1>(itNextLevel->second, key);
				}
			}

			return nullptr;
		}

		// Insert. 
		template<typename Lvl, typename K, size_t depth = 0>
		auto Insert(Lvl& lvl, const K& key)
		{
			auto&& levelKey = K::Getter<depth>::Get(key);

			if constexpr (Lvl::isLeaf)
			{
				return lvl.Insert(levelKey);
			}
			else
			{
				auto insNextLevel = lvl.Insert(levelKey);
				// Regardless of whether the level was inserted or already exists, call recursively
				return Insert<decltype(insNextLevel.first->second), K, depth + 1>(insNextLevel.first->second, key);
			}
		}

		// Remove
		// if eraseEmpty is true, higher levels with no elements are erased
		template<typename Lvl, typename K, size_t depth = 0>
		auto Erase(Lvl& lvl, const K& key, bool eraseEmpty)
		{
			auto&& levelKey = K::Getter<depth>::Get(key);

			if constexpr (Lvl::isLeaf)
			{
				return lvl.Erase(levelKey);
			}
			else
			{
				auto itNextLevel = lvl.Find(levelKey);

				if (itNextLevel != lvl.End())
				{
					size_t elementsErased = Erase<decltype(itNextLevel->second), K, depth + 1>(itNextLevel->second, key, eraseEmpty);

					if (eraseEmpty
						&& elementsErased > 0
						&& itNextLevel->second.IsEmpty())
					{
						lvl.Erase(itNextLevel);
					}

					return elementsErased;
				}
			}

			return 0;
		}


		// Iterate
		template<typename Lvl, typename F, typename ... Keys>
		bool IterateChildren(const Lvl& lvl, F&& func, Keys&& ...keys)
		{
			for (auto itLevel = lvl.Begin(); itLevel != lvl.End(); ++itLevel)
			{
				const auto& levelKey = itLevel->first;

				if constexpr (Lvl::isLeaf)
				{
					if (!func(std::forward<Keys>(keys)..., levelKey, itLevel->second))
					{
						return false;
					}
				}
				else
				{
					// Recursion again
					if (!IterateChildren(itLevel->second,
						std::forward<F>(func),
						std::forward<Keys>(keys)...,
						levelKey))
					{
						return false;
					}
				}
			}
			return true;
		}
	}


}