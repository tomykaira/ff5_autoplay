#include <iostream>
#include <unistd.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"
#include "dungeon/Map.hpp"
#include "dungeon/Symbol.hpp"
#include "dungeon/Edge.hpp"

namespace dungeon {

Map::Map(const cv::Mat groundTemplate) :
	myX(SIZE/2),
	myY(SIZE/2),
	map(boost::extents[SIZE][SIZE]),
	groundTemplate(groundTemplate.clone())
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			map[y][x] = new Unseen(this);
		}
	}

	auto firstFloor = new Floor(this);
	firstFloor->visit();
	replace(myX, myY, firstFloor);
}

Map::~Map()
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			delete map[y][x];
		}
	}
}


void Map::move(cv::Mat * rawImage, Direction direction)
{
	int nextX = myX + dx(direction), nextY = myY + dy(direction);
	if (Door * door = dynamic_cast<Door *>(map[nextY][nextX])) {
		if (door->isOpen()) {
			switch (moveTo(rawImage, direction)) {
			case NOT_MOVED:
				assert(false && "Door should be movable");
				break;
			case MOVED:
				replace(nextX, nextY, new Floor(this));
				break;
			case MAP_CHANGED:
				Link * link = new Link(this);
				replace(nextX, nextY, link);
				break;
			}
		} else {
			// Push the door
			moveTo(rawImage, direction);
			door->open();
		}
	} else if (moveTo(rawImage, direction) == MOVED) {
		myX = nextX;
		myY = nextY;
	} else {
		replace(nextX, nextY, new Block(this));
	}
}


void Map::detectSymbols(cv::Mat mat)
{
	cv::Mat topTemplate    = groundTemplate(cv::Rect(0,  0, 32, 16));
	cv::Mat bottomTemplate = groundTemplate(cv::Rect(0, 16, 32, 16));

	cv::Mat result;
	double topScore, bottomScore;

	for (int y = 1; y < 14; ++y) {
		for (int x = 0; x < 15; ++x) {
			cv::Rect grid(x*32, y*32 - 2, 32, 32);
			cv::Mat part = mat(grid), tallPart = mat(cv::Rect(x*32, y*32 - 18, 32, 48));

			cv::matchTemplate(part, topTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &topScore, NULL, NULL);

			cv::matchTemplate(part, bottomTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &bottomScore, NULL, NULL);

			if (topScore > 0.95 || bottomScore > 0.95) {
				cv::rectangle(mat, grid, cv::Scalar(255, 0, 0), CV_FILLED);
			}

			if (map[myY + (y - 7)][myX + (x - 7)]->isUnknown()) {
				if (x == 7 && y == 7) {
					replace(myX, myY, new Floor(this));
				} else if (topScore > 0.95 || bottomScore > 0.95) {
					replace(myX + (x - 7), myY + (y - 7), new Floor(this));
				} else if (isBackground(part)) {
					replace(myX + (x - 7), myY + (y - 7), new Block(this));
				} else if (isBackLink(part)) {
					// This condition is ambiguous
					if (map[myY + (y - 7) - 1][myX + (x - 7) - 1]->movable() ||
					    map[myY + (y - 7) - 1][myX + (x - 7) + 1]->movable()) {
						replace(myX + (x - 7), myY + (y - 7), new Floor(this));
						replace(myX + (x - 7), myY + (y - 7) + 1, new Link(this));
					} else {
						replace(myX + (x - 7), myY + (y - 7), new Link(this));
					}
				} else if (isUpSteps(tallPart)) {
					replace(myX + (x - 7), myY + (y - 7), new Link(this));
				} else if (isClosedDoor(tallPart)) {
					replace(myX + (x - 7), myY + (y - 7), new Door(this, CLOSED));
				} else if (isOpenDoor(tallPart)) {
					replace(myX + (x - 7), myY + (y - 7), new Door(this, OPEN));
				} else {
					replace(myX + (x - 7), myY + (y - 7), new Unidentified(this));
				}
			}
		}
	}
}


EdgeSet Map::subedges(Edge * edge, Direction direction)
{
	EdgeSet edges;

	int x = edge->x, y = edge->y;
	Edge * e;

	for (int i = 1; i < edge->length; ++i) {
		e = createEdge(x, y, direction);
		edges.push_back(std::shared_ptr<Edge>(e));

		x += dx(edge->direction);
		y += dy(edge->direction);
	}

	return edges;
}


EdgeSet Map::maximalEdges(EdgeSet edges)
{
	EdgeSet results;

	auto it = edges.begin();
	while (it != edges.end()) {
		bool pre_up, post_down;
		if (it - 1 < edges.begin()) {
			pre_up = true;
		} else {
			pre_up = (*it)->length - (*(it-1))->length > 0;
		}

		if (it + 1 >= edges.end()) {
			post_down = true;
		} else {
			post_down = (*it)->length - (*(it+1))->length > 0;
		}

		if (pre_up && post_down) {
			results.push_back(*it);
		}

		++it;
	}

	return results;
}

void Map::calculatePath()
{
	for (int i = LEFT; i <= DOWN; ++i) {
		auto d = static_cast<Direction>(i);

		Edge * edge = createEdge(myX, myY, d);

		if (edge->length != 0) {
			edge->addSubEdges(maximalEdges(subedges(edge, PREV(d))));
			edge->addSubEdges(maximalEdges(subedges(edge, NEXT(d))));
			rootEdges.push_back(std::shared_ptr<Edge>(edge));
		}
	}
}


void Map::debug()
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			if (x == myX && y == myY)
				std::cout << "@";
			else {
				bool onRootEdge = false, onSubEdge = false;

				auto it = rootEdges.begin();
				while (it != rootEdges.end()) {
					if ((*it)->include(x, y))
						onRootEdge = true;

					if (!onRootEdge) {
						auto sub = (*it)->getSubEdges();
						auto it_ = sub.begin();
						while (it_ != sub.end()) {
							if ((*it_)->include(x, y))
								onSubEdge = true;
							++it_;
						}
					}
					++it;
				}

				if (onRootEdge)
					std::cout << "1";
				else if (onSubEdge)
					std::cout << "2";
				else
					std::cout << map[y][x]->character();
			}
		}
		std::cout << std::endl;
	}
	for_each(rootEdges.begin(), rootEdges.end(), [](std::shared_ptr<Edge> e) { std::cout << (*e) << std::endl; });
}


void Map::replace(int x, int y, Symbol * s)
{
	delete map[y][x];
	map[y][x] = s;
}


Edge * Map::createEdge(int x, int y, Direction d)
{
	auto length = 0;
	const auto startX = x, startY = y;

	while (map[y][x]->movable()) {
		x += dx(d);
		y += dy(d);
		length++;
	}

	return new Edge(startX, startY, d, length);
}

} // dungeon
