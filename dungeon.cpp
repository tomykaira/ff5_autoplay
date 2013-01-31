#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "dungeon.hpp"
#include "dbus_client.hpp"
#include "recognition.hpp"

int dx(Direction d)
{
	if (d == LEFT)
		return -1;
	else if (d == RIGHT)
		return 1;
	else
		return 0;
}

int dy(Direction d)
{
	if (d == UP)
		return -1;
	else if (d == DOWN)
		return 1;
	else
		return 0;
}

/**
 * stream output operator
 */
std::string stringDirection (const Direction d) {
	switch (d) {
	case LEFT:
		return "LEFT";
	case UP:
		return "UP";
	case RIGHT:
		return "RIGHT";
	case DOWN:
		return "DOWN";
	}
	return "";
}


void drawGrid(cv::Mat mat)
{
	for (int y = 0; y < 448; y += 32) {
		for (int x = 0; x < 496; x += 32) {
			cv::rectangle(mat, cv::Rect(x, y, 32, 32), cv::Scalar(255, 0, 0), 1);
		}
	}
}


cv::Mat cropGroundTemplate(cv::Mat mat)
{
	return mat(cv::Rect(7*32, 8*32 - 2, 32, 32));
}


double difference(cv::Mat a, cv::Mat b)
{
	cv::Mat diff;
	cv::absdiff(a, b, diff);

	int changedPixels = 0;
	cv::Mat_<uint32_t>::iterator it = diff.begin<uint32_t>();
	for(; it!=diff.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) != 0)
			changedPixels++;
	}

	return (double)changedPixels / (double)(diff.rows * diff.cols);
}

MoveResult moveTo(cv::Mat * rawImage, Direction direction)
{
	static const int MAX_WAIT_COUNT = 50;

	std::string button;

	switch (direction) {
	case LEFT:
		button = "Left";
		break;
	case UP:
		button = "Up";
		break;
	case RIGHT:
		button = "Right";
		break;
	case DOWN:
		button = "Down";
		break;
	}

	cv::Rect fromRect(32 + dx(direction)*16,
	                  16 + dy(direction)*16,
	                  448, 416);
	cv::Rect   toRect(32 - dx(direction)*16,
	                  16 - dy(direction)*16,
	                  448, 416);

	cv::Mat expected = (*rawImage)(fromRect).clone();
	int waitCount = 0;

	dbusCallMethod(true, button.c_str());

	while (difference((*rawImage)(toRect), expected) > 0.1 && waitCount < MAX_WAIT_COUNT) {
		waitCount++;
		usleep(1000);
	}

	return difference((*rawImage)(toRect), expected) <= 0.1 ? MOVED : NOT_MOVED;
}

bool isBackground(cv::Mat crop)
{
	static const cv::Mat blackTemplate(32, 32, CV_8UC3, cv::Scalar(8, 8, 16));

	assert(crop.rows == 32 && crop.cols == 32);

	return (difference(crop, blackTemplate) < 0.05);
}

bool isUpSteps(cv::Mat crop)
{
	static const cv::Mat blackTemplate(16, 32, CV_8UC3, cv::Scalar(8, 8, 16));

	assert(crop.rows == 48 && crop.cols == 32);

	if (difference(crop(cv::Rect(0, 0, 32, 16)), blackTemplate) > 0.1) {
		return false;
	}

	for (int nth = 0; nth < 2; ++nth) {
		cv::Mat side = crop(cv::Rect(0, 24 + nth*16, 32, 8));
		int baseBlackPixels = 0;

		auto it = side.begin<uint32_t>();
		for(; it!=side.end<uint32_t>(); ++it) {
			uint32_t color = (*it & 0xffffff);
			if (color == 0x100808 || color == 0x080810)
				baseBlackPixels++;
		}

		// max: 256
		if (baseBlackPixels < 200) {
			return false;
		}
	}

	return true;
}

static const cv::Mat doorTemplate = cv::imread("templates/door.bmp", 1);

bool isDoor(cv::Mat crop)
{
	return templateIn(crop, cv::Rect(0, 0, 32, 48), doorTemplate);
}
