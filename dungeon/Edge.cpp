#include <sstream>
#include <boost/format.hpp>

#include "dungeon.hpp"
#include "dungeon/Edge.hpp"

namespace dungeon
{

Edge::Edge(int x, int y, Direction direction, int length) :
	x(x),
	y(y),
	direction(direction),
	length(length)
{
}

void Edge::addSubEdges(EdgeSet set) {
	for_each(set.begin(), set.end(), [&](std::shared_ptr<Edge> e) {addSubEdge(e);});
}

void Edge::addSubEdge(std::shared_ptr<Edge> e)
{
	if (e->x == x && e->y == y)
		return;
	subedges.push_back(e);
}

void Edge::addSubEdge(Edge * e)
{
	addSubEdge(std::shared_ptr<Edge>(e));
}

bool Edge::include(int px, int py) const
{
	int diffX = px - x;
	int diffY = py - y;
	int prodX = diffX * dx(direction);
	int prodY = diffY * dy(direction);

	return (dx(direction) == 0 && diffX == 0 && prodY > 0 && prodY < length)
		|| (dy(direction) == 0 && diffY == 0 && prodX > 0 && prodX < length);
}

Edge::operator std::string() const {
	std::stringstream ss;
	ss << boost::format("Edge(%d, %d, %s, %d)") % x % y % stringDirection(direction) % length << "{" << std::endl;

	auto it = subedges.begin();
	while (it != subedges.end()) {
		ss << **it;
		++it;
	}

	ss << "}" << std::endl;

  	return ss.str();
}

/**
 * stream output operator
 */
std::ostream& operator<<(std::ostream& lhs, const Edge& rhs) {
	lhs << std::string(rhs);
	return lhs;
}


} // dungeon
