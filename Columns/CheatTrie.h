#pragma once

#include <cinttypes>
#include <unordered_map>
#include <vector>
#include <optional>

namespace geng
{

	// A trie that tracks cheats as sequences of inputs, stored in an indexed vector
	// The index "0" always corresponds to the root.  It may succeed no one and when
	// it is hit, the trace has failed

	using CheatKey = uint32_t;
	using TrieIndex = size_t;

	constexpr TrieIndex CHEAT_TRIE_ROOT = 0;

	class CheatTrie
	{
	private:
		struct TrieNode
		{
			std::optional<CheatKey> key{};
			// One could of course use an array.  But that's a lot of useless bloat
			// An unordered_map from chars is a good compromise
			std::unordered_map<char,TrieIndex> m_nextNodes;
		};
	public:
		CheatTrie();

		TrieIndex AddEntry(const char* pEntry, CheatKey key);
		bool HasKey(TrieIndex idx) const
		{
			return idx < m_nodes.size() && m_nodes[idx].key.has_value();
		}

		CheatKey GetKey(TrieIndex idx) const
		{
			return *(m_nodes[idx].key);
		}

		TrieIndex GetNext(TrieIndex cur, char cNext) const;

	private:
		std::vector<TrieNode>   m_nodes;
	};

	
}