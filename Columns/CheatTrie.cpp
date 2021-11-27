#include "CheatTrie.h"

geng::CheatTrie::CheatTrie()
{
	m_nodes.emplace_back();
}

geng::TrieIndex geng::CheatTrie::AddEntry(const char* pEntry, geng::CheatKey cheatKey)
{
	const char* pC = pEntry;

	TrieIndex curNodeIndex = 0;
	while (*pC != '\0')
	{
		char curChar = *pC;
		if (curNodeIndex >= m_nodes.size())
		{
			return CHEAT_TRIE_ROOT;
		}

		TrieNode& curNode = m_nodes[curNodeIndex];

		auto itNext = curNode.m_nextNodes.find(curChar);
		if (itNext != curNode.m_nextNodes.end())
		{
			curNodeIndex = itNext->second;
		}
		else
		{
			TrieIndex nextNodeIndex = m_nodes.size();
			m_nodes.emplace_back();
			// Need to use the full call as the vector may grow
			m_nodes[curNodeIndex].m_nextNodes.emplace(curChar, nextNodeIndex);
			curNodeIndex = nextNodeIndex;
		}

		++pC;
	}

	m_nodes[curNodeIndex].key.emplace(cheatKey);
	return curNodeIndex;
}

geng::TrieIndex geng::CheatTrie::GetNext(TrieIndex curNodeIndex, char cNext) const
{
	if (curNodeIndex >= m_nodes.size())
	{
		return CHEAT_TRIE_ROOT;
	}

	auto itNext = m_nodes[curNodeIndex].m_nextNodes.find(cNext);

	return itNext != m_nodes[curNodeIndex].m_nextNodes.end() 
		? itNext->second : 
		CHEAT_TRIE_ROOT;
}