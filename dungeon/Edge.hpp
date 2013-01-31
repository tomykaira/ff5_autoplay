#include <vector>

#include "dungeon.hpp"

namespace dungeon
{

class Edge {
	EdgeSet subedges;

public:

	const int x;
	const int y;
	const Direction direction;
	const int length;

	Edge(int x, int y, Direction direction, int length);

	operator std::string() const;

	void addSubEdges(EdgeSet set);

	void addSubEdge(Edge * e);
	void addSubEdge(std::shared_ptr<Edge> e);

	const EdgeSet getSubEdges() const {
		return subedges;
	}

	bool include(int px, int py) const;
};

std::ostream& operator<<(std::ostream& lhs, const Edge& rhs);

} // dungeon
