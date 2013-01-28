#include <boost/optional.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "recognition.hpp"

static const cv::Mat TEMPLATE_ATTACK = cv::imread("templates/attack.bmp", 1);
static const cv::Mat TEMPLATE_INDEX = cv::imread("templates/index.bmp", 0);

// cv::Rect(16, 0, 480, 448)    : full play area
// cv::Rect(16, 320, 480, 120)  : command area (outer bound)
// cv::Rect(16, 320, 96, 120)   : enemy name
// cv::Rect(112, 320, 112, 120) : command area
// cv::Rect(224, 320, 272, 120) : character status
// cv::Rect(332, 320, 72, 120)  : hp area

int attackCommandIsDisplayed(cv::Mat mat)
{
	cv::Mat commandArea = mat(cv::Rect(112, 320, 112, 120));
	cv::Mat result;

	cv::matchTemplate(commandArea, TEMPLATE_ATTACK, result, CV_TM_CCOEFF_NORMED);

	double maxScore;
	cv::Point point;
	cv::Rect roi(0, 0, TEMPLATE_ATTACK.cols, TEMPLATE_ATTACK.rows);
	cv::minMaxLoc(result, NULL, &maxScore, NULL, &point);
	roi.x = point.x;
	roi.y = point.y;

	return maxScore > 0.95;
}

int markActiveCharacter(cv::Mat mat)
{

	const cv::Scalar yellow = cv::Scalar(0, 255, 255);
	const cv::Scalar blue   = cv::Scalar(255, 0, 0);

	for (int i = 0; i < 4; ++i) {
		cv::Rect nameRect = cv::Rect(224, 334 + i*24, 72, 16);
		cv::Mat nameArea = mat(nameRect);
		int yellowCount = 0;

		cv::Mat_<uint32_t>::iterator it = nameArea.begin<uint32_t>();
		for(; it!=nameArea.end<uint32_t>(); ++it) {
			if (((*it & 0xff0000) >> 16) > 100 && (*it & 0xff) < 100)
				++ yellowCount;
		}

		cv::rectangle(mat, nameRect, yellowCount > 100 ? yellow : blue, 1);

		if (yellowCount > 100)
			return i;
	}

	return -1;

}

boost::optional<cv::Point> findIndexLocation(cv::Mat mat)
{
	std::vector<cv::Mat> input(3), zeros(3);
	cv::Mat black, result;

	cv::split(mat, input);

	for (int i = 0; i < 3; ++i) {
		cv::threshold(input[i], zeros[i], 0, 255, cv::THRESH_BINARY_INV);
	}

	cv::bitwise_and(zeros[0], zeros[1], black);
	cv::bitwise_and(black, zeros[2], black);

	double maxScore;
	cv::Point point;
	cv::matchTemplate(black, TEMPLATE_INDEX, result, CV_TM_CCOEFF_NORMED);
	cv::minMaxLoc(result, NULL, &maxScore, NULL, &point);

	if (maxScore > 0.8)
		return point;
	else
		return boost::none;
}
