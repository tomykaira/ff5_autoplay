#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"
#include "dbus_client.hpp"
#include "dungeon/LefthandMethod.hpp"

namespace dungeon {

void LefthandMethod::clearCharacter(cv::Mat mat)
{
	cv::rectangle(mat, PLAYER_RECT, cv::Scalar(255, 0, 0), CV_FILLED);
}

LefthandMethod::LefthandMethod() :
	PLAYER_RECT(cv::Rect(7*32, 7*32 - 4, 32, 32)),
	MOVE_THRESHOLD(10000),
	currentDirection(LEFT)
{
}

void LefthandMethod::tryDirection(Direction direction)
{
	std::cout << "Try " << direction << std::endl;
	switch (direction) {
	case LEFT:
		dbusCallMethod(true, "Left");
		break;
	case UP:
		dbusCallMethod(true, "Up");
		break;
	case RIGHT:
		dbusCallMethod(true, "Right");
		break;
	case DOWN:
		dbusCallMethod(true, "Down");
		break;
	}
	currentDirection = direction;
	usleep(1000*1000);
}

bool LefthandMethod::tryStep(cv::Mat mat)
{
	cv::Mat current = mat.clone(), diff;
	clearCharacter(current);

	if (previous.empty()) {
		previous = current;
		return true;
	}

	cv::absdiff(current, previous, diff);

	int changedPixels;
	cv::Mat_<uint32_t>::iterator it = diff.begin<uint32_t>();
	for(; it!=diff.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) != 0)
			changedPixels++;
	}

	previous = current;

	if (changedPixels > MOVE_THRESHOLD) {
		tryDirection(PREV(currentDirection));
		return true;
	} else {
		tryDirection(NEXT(currentDirection));
		return false;
	}
}


}
