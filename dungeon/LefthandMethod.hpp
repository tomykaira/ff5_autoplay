#ifndef _LEFTHANDMETHOD_H_
#define _LEFTHANDMETHOD_H_

#include "dungeon.hpp"

namespace dungeon {

class LefthandMethod{
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

}

#endif /* _LEFTHANDMETHOD_H_ */
