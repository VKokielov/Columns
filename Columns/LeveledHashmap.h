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
	class LeveledHashmap
	{
	public:
		constexpr bool isLeaf = sizeof...(RestKeys) == 0;
		using TElement = std::conditional_t < isLeaf,
			ValueType,
			LeveledHashMap<ValueType, RestKeys...>;
		using TValue = ValueType;
		using TIter = typename std::unordered_map<FirstKey, TElement>::iterator;

		LeveledHasmap() = default;

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
		auto Find(const FirstKey& key) const
		{
			return m_childMap.find(key);
		}

		// Iterate
		auto Begin()
		{
			return std::begin(m_childMap);
		}

		auto CBegin() const
		{
			return std::cbegin(m_childMap);
		}

		auto End()
		{
			return std::begin(m_childMap);
		}

		auto CEnd() const
		{
			return std::cbegin(m_childMap);
		}

		std::size_t GetDepth() const { return m_depth; }
		bool IsEmpty() const { return m_childMap.empty(); }

	private:
		LeveledHashmap(std::size_t depth)
			:m_depth(depth)
		{ }

		std::unordered_map<FirstKey, TElement> m_childMap;
		std::size_t m_depth{ 0 };
	};

	// Helper functions for operations on data

	// Find
	// It's not possible to return an iterator here, since "begin" and "end"
	// are ill-defined when there are many maps
	template<typename Lvl, typename K>
	typename Lvl::TValue* Find(const Lvl& lvl, const K& key)
	{
		const auto& levelKey = GetLevelKey(key, lvl.GetDepth());
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
			// "Linear recursion"
			auto itNextLevel = lvl.Find(levelKey);
			if (itNextLevel != lvl.End())
			{
				return Find(itNextLevel->second, key);
			}
		}

		return nullptr;
	}

	// Insert. 
	template<typename Lvl, typename K>
	auto Insert(Lvl& lvl, const K& key)
	{
		const auto& levelKey = GetLevelKey(key, lvl.GetDepth());
		
		if constexpr (Lvl::isLeaf)
		{
			return lvl.Insert(levelKey);
		}
		else
		{
			auto insNextLevel = lvl.Insert(levelKey);
			// Regardless of whether the level was inserted or already exists, call recursively
			return Insert(insNextLevel.first->second, key);
		}
	}

	// Remove
	// if eraseEmpty is true, higher levels with no elements are erased
	template<typename Lvl, typename K>
	auto Erase(Lvl& lvl, const K& key, bool eraseEmpty)
	{
		const auto& levelKey = GetLevelKey(key, lvl.GetDepth());

		if constexpr (Lvl::isLeaf)
		{
			return lvl.Erase(levelKey);
		}
		else
		{
			auto itNextLevel = lvl.Find(levelKey);

			if (itNextLevel != lvl.End())
			{
				size_t elementsErased = Erase(itNextLevel->second, key, eraseEmpty);

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
	void IterateChildren(const Lvl& lvl, F&& func, Keys&& ...keys)
	{
		for (auto itLevel = lvl.Begin(); itLevel != lvl.End(); ++itLevel)
		{
			const auto& levelKey = itLevel->first;

			if constexpr (Lvl::isLeaf)
			{
				func(std::forward<Keys>(keys)..., levelKey, itLevel->second);
			}
			else
			{
				// Recursion again
				IterateChildren(itLevel->second,
					std::forward<F>(func),
					std::forward<Keys>(keys)...,
					levelKey);
			}

		}
	}


}