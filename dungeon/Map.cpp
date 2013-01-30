#include <iostream>
#include <unistd.h>
#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"
#include "dungeon/Map.hpp"
#include "dungeon/Symbol.hpp"

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
	if (moveTo(rawImage, direction)) {
		myX += dx(direction);
		myY += dy(direction);
	} else {
		replace(myX + dx(direction), myY + dy(direction), new Block(this));
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
			cv::Mat part = mat(grid);

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
				} else {
					replace(myX + (x - 7), myY + (y - 7), new Unidentified(this));
				}
			}
		}
	}
}


void Map::debug()
{
	for (int y = 0; y < SIZE; ++y) {
		for (int x = 0; x < SIZE; ++x) {
      if (x == myX && y == myY)
        std::cout << "@";
      else
        std::cout << map[y][x]->character();
		}
		std::cout << std::endl;
	}
}


void Map::replace(int x, int y, Symbol * s)
{
	delete map[y][x];
	map[y][x] = s;
}

}
