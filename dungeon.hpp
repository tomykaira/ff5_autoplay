#ifndef _DUNGEON_H_
#define _DUNGEON_H_

#include <string>
#include <memory>

#include <boost/multi_array.hpp>
#include <boost/optional.hpp>

#include <opencv2/core/core.hpp>

#define PREV(x) (Direction(((x) + 3) % 4))
#define NEXT(x) (Direction(((x) + 1) % 4))

enum Direction {LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3};

class Symbol
{
protected:
	bool visited;

public:
	Symbol() : visited(false) {};

	virtual bool isVisited() const {
		return visited;
	}

	virtual void visit() {
		visited = true;
	}

	virtual bool isUnknown() const {
		return true;
	}

	virtual std::string character() const {
		return " ";
	}
};

class Map
{
	static const int SIZE = 32;

	int myX, myY;

	boost::multi_array<Symbol *, 2> map;

public:
	Map();
	virtual ~Map();
	void move(Direction direction);
	void detectSymbols(cv::Mat mat);

	void debug();
};

class Floor : public Symbol
{
public:
	bool isUnknown() const {
		return false;
	}

	std::string character() const {
		return "_";
	}
};

class Link : public Symbol
{
	boost::optional<Map> destination;

public:
	bool isUnknown() const {
		return false;
	}

	bool isVisited() const {
		assert((visited && destination) || ((!visited) && (!destination)));
		return visited;
	}

	void visit() {
		destination = Map();
	}

	Map getDestination() const {
		assert(destination);
		return *destination;
	}

	std::string character() const {
		return "*";
	}
};





class LefthandMethod
{
	const cv::Rect PLAYER_RECT;
	const int MOVE_THRESHOLD;

	cv::Mat previous;
	Direction currentDirection;

	void tryDirection(Direction direction);
	void clearCharacter(cv::Mat mat);

public:
	LefthandMethod();
	bool tryStep(cv::Mat mat);
	virtual ~LefthandMethod() {};
};

void drawGrid(cv::Mat mat);

#endif /* _DUNGEON_H_ */
