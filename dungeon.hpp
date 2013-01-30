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

namespace dungeon {
class Map;
class Symbol;

class LefthandMethod;
}

void drawGrid(cv::Mat mat);
cv::Mat cropGroundTemplate(cv::Mat mat);

#endif /* _DUNGEON_H_ */
