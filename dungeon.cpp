#include "dungeon.hpp"

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
