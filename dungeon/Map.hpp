#ifndef _MAP_H_
#define _MAP_H_

#include "dungeon.hpp"

namespace dungeon {

class Map
{
	static const int SIZE = 32;

	int myX, myY;

	boost::multi_array<Symbol *, 2> map;

public:
	const cv::Mat groundTemplate;

	Map(const cv::Mat groundTemplate);
	virtual ~Map();
	void move(Direction direction);
	void detectSymbols(cv::Mat mat);

	void debug();
};

}

#endif /* _MAP_H_ */