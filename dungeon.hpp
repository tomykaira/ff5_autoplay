#ifndef _DUNGEON_H_
#define _DUNGEON_H_

#include <opencv2/core/core.hpp>

#define PREV(x) (Direction(((x) + 3) % 4))
#define NEXT(x) (Direction(((x) + 1) % 4))

enum Direction {LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3};

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
void markGround(cv::Mat mat);

#endif /* _DUNGEON_H_ */
