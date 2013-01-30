#include <boost/optional.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "recognition.hpp"

static const cv::Mat TEMPLATE_ATTACK = cv::imread("templates/attack.bmp", 1);
static const cv::Mat TEMPLATE_INDEX = cv::imread("templates/index.bmp", 0);
static const cv::Mat TEMPLATE_WON = cv::imread("templates/won.bmp", 1);

bool templateIn(cv::Mat mat, cv::Rect roi, const cv::Mat temp)
{
	cv::Mat result;
	cv::matchTemplate(mat(roi), temp, result, CV_TM_CCOEFF_NORMED);

	double maxScore;
	cv::minMaxLoc(result, NULL, &maxScore, NULL, NULL);

	return maxScore > 0.95;
}

// cv::Rect(16, 0, 480, 448)    : full play area
// cv::Rect(16, 320, 480, 120)  : command area (outer bound)
// cv::Rect(16, 320, 96, 120)   : enemy name
// cv::Rect(112, 320, 112, 120) : command area
// cv::Rect(224, 320, 272, 120) : character status
// cv::Rect(332, 320, 72, 120)  : hp area

bool inBattle(cv::Mat mat)
{
	cv::Mat battleGraphicArea = mat(cv::Rect(16, 0, 480, 320));
	cv::Mat battleInfoArea = mat(cv::Rect(16, 320, 480, 120));

	int graphicCount = 0, infoCount = 0;

	cv::Mat_<uint32_t>::iterator it = battleGraphicArea.begin<uint32_t>();
	for(; it!=battleGraphicArea.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) == 128)
			++ graphicCount;
	}

	it = battleInfoArea.begin<uint32_t>();
	for(; it!=battleInfoArea.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) == 128)
			++ infoCount;
	}

	return infoCount > 30000 && graphicCount < 5000;
}

bool inField(cv::Mat mat)
{
	cv::Mat battleGraphicArea = mat(cv::Rect(16, 0, 480, 320));
	cv::Mat battleInfoArea = mat(cv::Rect(16, 320, 480, 120));

	int graphicCount = 0, infoCount = 0, notBlack = 0;

	cv::Mat_<uint32_t>::iterator it = battleGraphicArea.begin<uint32_t>();
	for(; it!=battleGraphicArea.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) == 128)
			++ graphicCount;
		if ((*it & 0xffffff) != 0)
			++ notBlack;
	}

	it = battleInfoArea.begin<uint32_t>();
	for(; it!=battleInfoArea.end<uint32_t>(); ++it) {
		if ((*it & 0xffffff) == 128)
			++ infoCount;
		if ((*it & 0xffffff) != 0)
			++ notBlack;
	}

	return infoCount < 5000 && graphicCount < 5000 && notBlack > 10240;
}

bool afterBattle(cv::Mat mat)
{
	return templateIn(mat, cv::Rect(48, 26, 170, 24), TEMPLATE_WON);
}

bool attackCommandIsDisplayed(cv::Mat mat)
{
	return templateIn(mat, cv::Rect(112, 320, 112, 120), TEMPLATE_ATTACK);
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

	if (maxScore > 0.7)
		return point;
	else
		return boost::none;
}
