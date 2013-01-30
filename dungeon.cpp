#include <iostream>
#include <unistd.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"
#include "dbus_client.hpp"

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

////////////////
// Map

Map::Map() :
	myX(SIZE/2),
	myY(SIZE/2),
	map(boost::extents[SIZE][SIZE])
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			map[y][x] = new Symbol();
		}
	}

	auto firstFloor = new Floor();
	firstFloor->visit();
	map[myY][myX] = firstFloor;
}

Map::~Map()
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			delete map[y][x];
		}
	}
}


void Map::move(Direction direction)
{
	switch (direction) {
	case LEFT:
		dbusCallMethod(true, "Left");
		myX--;
		break;
	case UP:
		dbusCallMethod(true, "Up");
		myY--;
		break;
	case RIGHT:
		dbusCallMethod(true, "Right");
		myX++;
		break;
	case DOWN:
		dbusCallMethod(true, "Down");
		myY++;
		break;
	}
	usleep(1000*1000);
}


void Map::detectSymbols(cv::Mat mat)
{
	cv::Mat groundTemplate = mat(cv::Rect(7*32, 8*32 - 2, 32, 32)).clone();
	cv::Mat topTemplate    = groundTemplate(cv::Rect(0,  0, 32, 16));
	cv::Mat bottomTemplate = groundTemplate(cv::Rect(0, 16, 32, 16));

	cv::Mat result;
	double topScore, bottomScore;

	for (int y = 1; y < 14; ++y) {
		for (int x = 0; x < 15; ++x) {
			cv::Rect grid(x*32, y*32 - 2, 32, 32);
			cv::Mat part = mat(grid);

			cv::matchTemplate(part, topTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &topScore, NULL, NULL);

			cv::matchTemplate(part, bottomTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &bottomScore, NULL, NULL);

			if (topScore > 0.95 || bottomScore > 0.95) {
				if (map[myY + (y - 7)][myX + (x - 7)]->isUnknown()) {
					std::cout << "(" << myX + (x - 7) << ", " << myY + (y - 7) << ")" << std::endl;
					delete map[myY + (y - 7)][myX + (x - 7)];
					map[myY + (y - 7)][myX + (x - 7)] = new Floor();
				}
				cv::rectangle(mat, grid, cv::Scalar(255, 0, 0), CV_FILLED);
			}
		}
	}
 }


void Map::debug()
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			std::cout << map[y][x]->character();
		}
		std::cout << std::endl;
	}
}


void drawGrid(cv::Mat mat)
{
	for (int y = 0; y < 448; y += 32) {
		for (int x = 0; x < 496; x += 32) {
			cv::rectangle(mat, cv::Rect(x, y, 32, 32), cv::Scalar(255, 0, 0), 1);
		}
	}
}
