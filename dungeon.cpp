#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

#include "dungeon.hpp"

void drawGrid(cv::Mat mat)
{
	for (int y = 0; y < 448; y += 32) {
		for (int x = 0; x < 496; x += 32) {
			cv::rectangle(mat, cv::Rect(x, y, 32, 32), cv::Scalar(255, 0, 0), 1);
		}
	}
}

void markGround(cv::Mat mat)
{
	cv::Mat groundTemplate = mat(cv::Rect(7*32, 8*32 - 2, 32, 32)).clone();
	cv::Mat topTemplate    = groundTemplate(cv::Rect(0,  0, 32, 16));
	cv::Mat bottomTemplate = groundTemplate(cv::Rect(0, 16, 32, 16));


	cv::Mat result;
	double topScore, bottomScore;

	for (int y = 30; y < 448; y += 32) {
		for (int x = 0; x < 496; x += 32) {
			cv::Rect grid(x, y, 32, 32);
			cv::Mat part = mat(grid);

			cv::matchTemplate(part, topTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &topScore, NULL, NULL);

			cv::matchTemplate(part, bottomTemplate, result, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(result, NULL, &bottomScore, NULL, NULL);

			if (topScore > 0.95 || bottomScore > 0.95) {
				cv::rectangle(mat, grid, cv::Scalar(255, 0, 0), CV_FILLED);
			}
		}
	}
}
