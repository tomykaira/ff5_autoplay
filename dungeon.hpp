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

int dx(const Direction d);
int dy(const Direction d);
std::string stringDirection (const Direction d);

namespace dungeon {
class Map;
class Symbol;
class Edge;
typedef std::vector<std::shared_ptr<Edge>> EdgeSet;

class LefthandMethod;
}

void drawGrid(cv::Mat mat);
cv::Mat cropGroundTemplate(cv::Mat mat);

enum MoveResult {NOT_MOVED, MOVED, MAP_CHANGED};

MoveResult moveTo(cv::Mat * rawImage, Direction direction);

// crop: 32x32
bool isBackground(cv::Mat crop);

// crop: 32x48
bool isUpSteps(cv::Mat crop);
bool isClosedDoor(cv::Mat crop);
bool isOpenDoor(cv::Mat crop);

#endif /* _DUNGEON_H_ */
