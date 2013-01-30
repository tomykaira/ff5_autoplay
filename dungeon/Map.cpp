#include <iostream>
#include <unistd.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"
#include "dungeon/Map.hpp"
#include "dungeon/Symbol.hpp"
#include "dbus_client.hpp"

namespace dungeon {

Map::Map(const cv::Mat groundTemplate) :
	myX(SIZE/2),
	myY(SIZE/2),
	map(boost::extents[SIZE][SIZE]),
	groundTemplate(groundTemplate.clone())
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
			map[y][x] = new Symbol(this);
		}
	}

	auto firstFloor = new Floor(this);
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
					delete map[myY + (y - 7)][myX + (x - 7)];
					map[myY + (y - 7)][myX + (x - 7)] = new Floor(this);
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

}
