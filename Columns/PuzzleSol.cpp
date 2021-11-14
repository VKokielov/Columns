
/*
struct Vertex
{
	unsigned int idx;
	unsigned int originalPt;
	unsigned int xorig, yorig;

	unordered_set<unsigned int> neighbors;

	Vertex(unsigned int idx_, unsigned int orignalPt_,
							  unsigned int xorig_, unsigned int yorig_)
		:idx(idx_),
		originalPt(orignalPt_),
		xorig(xorig_),
		yorig(yorig_)
	{ }

	bool isNeighbor(unsigned int nghbr) const
	{
		return neighbors.count(nghbr) > 0;
	}

	void getNeighbors(vector<unsigned int>& recipient) const
	{
		for (unsigned int nghbr : neighbors)
		{
			recipient.push_back(nghbr);
		}
	}

	void addNeighbor(unsigned int vx)
	{
		neighbors.insert(vx);
	}
};

bool isVH(const Vertex& vxA, const Vertex& vxB)
{
	return vxA.xorig == vxB.xorig || vxA.yorig == vxB.yorig;
}

unsigned int dfsFindCycles(const vector<Vertex>& graph,
	unsigned int vxStart)
{

	struct DFSState
	{
		unsigned int vx;
		unsigned int idxN;
		std::vector<unsigned int> neighbors;

		DFSState(const vector<Vertex>& graph, unsigned int vx_)
			:idxN(0),
			vx(vx_)
		{
			graph[vx].getNeighbors(neighbors);
		}

		bool hasNeighbor() const { return idxN < neighbors.size(); }

		unsigned int nextNeighbor()
		{
			return neighbors[idxN++];
		}
	};

	unsigned int nCycles = 0;
	unordered_set<unsigned int> visited;
	std::vector<DFSState>   dfsState;

	dfsState.emplace_back(graph, vxStart);
	visited.insert(vxStart);

	while (!dfsState.empty())
	{
		// Get the next neighbor
		if (dfsState.back().hasNeighbor())
		{
			unsigned int nextNeighbor = dfsState.back().nextNeighbor();

			const Vertex& nextVertex = graph[nextNeighbor];

			if (dfsState.size() >= 2)
			{
				// Get the correspondent
				unsigned int correspondent = dfsState[dfsState.size() - 2].vx;

				if (!isVH(graph[correspondent], nextVertex))
				{
					continue;
				}
			}

			if (dfsState.size() == 4)
			{
				if (nextNeighbor == vxStart)
				{
					std::cout << "Found cycle:\n";
					for (size_t i = 0; i < dfsState.size(); ++i)
					{
						std::cout << graph[dfsState[i].vx].originalPt << "--";
					}
					std::cout << "\n--------\n";
					++nCycles;
				}
			}
			else
			{
				// Push or skip?
				if (!visited.count(nextNeighbor))
				{
					// Push
					visited.insert(nextNeighbor);
					dfsState.emplace_back(graph, nextNeighbor);
				}
			}
		}
		else // pop
		{
			visited.erase(dfsState.back().vx);
			dfsState.pop_back();
		}
	}

	return nCycles;
}

unsigned int allDiamonds(const vector<Vertex>& graph)
{
	unsigned int fourCycleCount = 0;
	for (unsigned int vxIndex = 0; vxIndex < graph.size(); ++vxIndex)
	{
		fourCycleCount += dfsFindCycles(graph, vxIndex);
	}

	// Each four-cycle is counted four times
	return fourCycleCount / 8;
}

struct MappedGraph
{
	vector<Vertex>   graph;
	unordered_map<unsigned int, unsigned int>
		graphMap;

	unsigned int getVertex(unsigned int pt, unsigned int xorig, unsigned int yorig)
	{
		if (graphMap.count(pt))
		{
			return graphMap[pt];
		}

		unsigned int vx = graph.size();
		graphMap[pt] = vx;
		graph.emplace_back(vx,pt, xorig, yorig);
		return vx;
	}

	void addPair(unsigned int ptA,
		unsigned int xA,
		unsigned int yA,
		unsigned int ptB,
		unsigned int xB,
		unsigned int yB)
	{
		unsigned int vxAid = getVertex(ptA, xA, yA);
		unsigned int vxBid = getVertex(ptB, xB, yB);

		Vertex& vxA = graph[vxAid];
		Vertex& vxB = graph[vxBid];

		vxA.addNeighbor(vxBid);
		vxB.addNeighbor(vxAid);
	}

};

int solution(vector<int> &X, vector<int> &Y) {
	// Construct a set of point graphs around
	// segments of equal lengths

	unordered_map<long, MappedGraph>  ptDistanceGraphMap;

	// Go through all segments and add them to the graph in question
	for (size_t i = 0; i < X.size(); ++i)
	{
		// Point 1:
		int x1 = X[i];
		int y1 = Y[i];
		for (size_t j = i + 1; j < X.size(); ++j)
		{
			int x2 = X[j];
			int y2 = Y[j];

			int diffX = x1 - x2;
			int diffY = y1 - y2;

			long sqDist = diffX * diffX + diffY * diffY;

			MappedGraph& mpGraph = ptDistanceGraphMap[sqDist];

			// Now add the point indices (segment) to the graph
			mpGraph.addPair(i, x1, y1, j, x2, y2);
		}
	}

	// Now for each distance, compute the number of diamonds
	// formed by points thus distant, and return the total
	unsigned int totalDiamonds = 0;
	for (const auto& mgraph : ptDistanceGraphMap)
	{
		std::cout << "Counting diamonds with dist " << mgraph.first << "\n";
		totalDiamonds += allDiamonds(mgraph.second.graph);
	}

	return totalDiamonds;
}
*/

/*

unsigned int countEquidistant(const std::vector<int>& coordList,
	 int pivot)
{
	auto itLB = std::lower_bound(coordList.begin(), coordList.end(), pivot);

	if (itLB == coordList.end())
	{
		// This means the pivot is above all elements
		return 0;
	}

	size_t idxLB = itLB - coordList.begin();

	int iiLB = (int)idxLB;

	int iiU = iiLB;
	if (coordList[iiU] == pivot)
	{
		++iiU;
	}

	int iiL = iiLB - 1;

	unsigned int nED = 0;
	while (iiL >= 0 && iiU < (int)coordList.size())
	{
		int ldiff = pivot - coordList[iiL];
		int udiff = coordList[iiU] - pivot;

		if (ldiff < udiff)
		{
			--iiL;
		}
		else if (ldiff > udiff)
		{
			++iiU;
		}
		else  // equal
		{
			++nED;
			--iiL;
			++iiU;
		}
	}

	return nED;
}

std::unordered_map<int, std::vector<int> >
buildGeomMap(const vector<int>& older,
	const vector<int>& younger)
{
	// NOTE:  Assumes older.size() == younger.size()

	std::unordered_map<int, std::vector<int> > retMap;

	for (size_t i = 0; i < older.size(); ++i)
	{
		retMap[older[i]].push_back(younger[i]);
	}

	for (auto& coordV : retMap)
	{
		std::sort(coordV.second.begin(), coordV.second.end());
	}

	return retMap;
}

int solution(vector<int> &X, vector<int> &Y)
{
	std::unordered_map<int, std::vector<int> > colMap = buildGeomMap(X, Y);
	std::unordered_map<int, std::vector<int> > rowMap = buildGeomMap(Y, X);

	unsigned int diamondCount = 0;

	for (const auto& column : colMap)
	{
		const std::vector<int>& ys = column.second;

		for (size_t i = 0; i < ys.size(); ++i)
		{
			int y1 = ys[i];

			for (size_t j = i + 1; j < ys.size(); ++j)
			{
				int y2 = ys[j];

				int diffy = y2 - y1;

				std::cout << "(" << column.first << "," << y1 << ") - (" << column.first << "," << y2 << ")\n";

				if ((diffy % 2) == 0)
				{
					int y34 = (y1 + y2) / 2;

					if (rowMap.count(y34) > 0)
					{
						const std::vector<int>& xs = rowMap[y34];
						diamondCount += countEquidistant(xs, column.first);
					}
				}

			}
		}
	}
	return diamondCount;

}

*/

/*
// you can use includes, for example:
// #include <algorithm>

// you can write to stdout for debugging purposes, e.g.
// cout << "this is a debug message" << endl;

#include <map>

int solution(vector<int> &A, int X) {

	// histogram...
	map<int,unsigned int> histF;
	for (int val : A)
	{
		++histF[val];
	}

	unsigned int validPairs = 0;
	for (const auto& shorterSide : histF)
	{
		// Need at least two to participate
		if (shorterSide.second < 2)
		{
			continue;
		}

		int minLongerSide = X/shorterSide.first;

		int minSelf = shorterSide.second >= 4 ? shorterSide.first
						 : shorterSide.first+1;

		int startingLonger = std::max(minLongerSide,minSelf);

		auto itLonger = histF.lower_bound(startingLonger);

		while (itLonger != histF.end())
		{
			if (itLonger->second >= 2)
			{
				++validPairs;
			}
			++itLonger;
		}
	}

	return (int)validPairs;
}
*/



/*
// you can use includes, for example:
// #include <algorithm>

// you can write to stdout for debugging purposes, e.g.
// cout << "this is a debug message" << endl;
unordered_map<int,unsigned int>
	buildHist(const vector<int>& vec)
{
	unordered_map<int,unsigned int> hist;
	for (int val : vec)
	{
		if (hist.count(val) > 0)
		{
			++hist[val];
		}
		else
		{
			hist[val] = 1;
		}
	}

	return hist;
}

unsigned int scourPairs(unordered_map<int,unsigned int>& hist,
	unsigned int* limit)
{
	if (limit && !(*limit))
	{
		return 0;
	}

	unsigned int pairCount = 0;
	for (auto it = hist.begin();
		  it != hist.end();
		  ++it)
	{
		unsigned int maxSocks = 0;

		// Only take an even number of socks

		if (!limit)
		{
			maxSocks = 2 * (it->second/2);
		}
		else
		{
			maxSocks = 2 * std::min(*limit / 2, it->second / 2);
		}

		pairCount += maxSocks / 2;

		it->second -= maxSocks;
		if (!it->second)
		{
			it = hist.erase(it);
		}

		if (limit)
		{
			*limit -= maxSocks;
			if (!(*limit))
			{
				break;
			}
		}
	}

	return pairCount;
}

unsigned int
	match(const unordered_map<int,unsigned int>& histC,
		  unordered_map<int,unsigned int>& histD,
		  unsigned int& limit)
{
	if (!limit)
	{
		return 0;
	}

	unsigned int pairCount = 0;
	for (auto& cleanColor : histC)
	{
		if (histD.count(cleanColor.first) > 0)
		{
			// Match.
			--histD[cleanColor.first];
			--limit;
			++pairCount;
		}

		if (!limit)
		{
			break;
		}
	}

	return pairCount;
}

int solution(int K, vector<int> &C, vector<int> &D) {
	auto histC = buildHist(C);
	auto histD = buildHist(D);

	unsigned int totalPairs = 0;
	unsigned int limit = (unsigned int)K;

	totalPairs += scourPairs(histC,nullptr);
	totalPairs += match(histC, histD, limit);
	totalPairs += scourPairs(histD, &limit);

	return totalPairs;
}
*/