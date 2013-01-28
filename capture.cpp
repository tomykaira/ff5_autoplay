/**
 * gcc capture_screen_x11-01.c -L/usr/X11/lib -lX11
 * http://d.hatena.ne.jp/sa-y/20070306
**/

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/time.h>

#include <boost/optional.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "xlib_ext.hpp"
#include "number.hpp"
#include "dbus_client.hpp"

#define DIFF(end, start) (((end).tv_sec - (start).tv_sec)*1000*1000 + (end).tv_usec - (start).tv_usec)

const cv::Mat TEMPLATE_ATTACK = cv::imread("templates/attack.bmp", 1);
const cv::Mat TEMPLATE_INDEX = cv::imread("templates/index.bmp", 0);

void after(const struct timeval * from, struct timeval * to, int diff)
{
	int new_usec = from->tv_usec + diff;
	to->tv_usec = new_usec % (1000*1000);
	to->tv_sec  = from->tv_sec + new_usec / (1000*1000);
}

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

int sendCommand(int activeCharacter, cv::Mat mat)
{
	cv::Mat hpArea = mat(cv::Rect(332, 320, 72, 120));
	std::vector<int> HPs = findNumbers(hpArea);

	std::vector<int>::iterator it = HPs.begin();
	while (it != HPs.end()) {
		std::cout << *it << " ";
		++it;
	}
	std::cout << std::endl;

	int lowestHP = 9999;
	int lowestHPCharacter = -1;

	for (int i = 0; i < HPs.size(); ++i) {
		if (HPs[i] < lowestHP && HPs[i] > 0) {
			lowestHP = HPs[i];
			lowestHPCharacter = i;
		}
	}

	std::cout << lowestHPCharacter << " : " << lowestHP << std::endl;

	switch (activeCharacter) {
	case 0:
		std::cout << "ファリス" << std::endl;
		attack();
		break;
	case 1: // レナ
		std::cout << "レナ" << std::endl;
		attackParty(lowestHPCharacter);
		break;
	case 2: // ガラフ
		std::cout << "ガラフ" << std::endl;
		attack();
		break;
	case 3: // バッツ
		std::cout << "バッツ" << std::endl;
		attack();
		break;
	}

	return 0;
}

int main(int argc, char* argv[])
{
	Display* display;
	int screen;
	Window rootWindow, targetWindow;
	XWindowAttributes win_info;
	XImage *image;
	const char * ffWindowName = "\"FINAL FANTASY 5\" Snes9x: Linux: 1.53";

	cv::Mat mat;

	int active;
	bool autoControl = true;

	display = XOpenDisplay("");
	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);

	targetWindow = windowWithName(display, rootWindow, ffWindowName);

	XGetWindowAttributes(display, targetWindow, &win_info);

	if (dbusInit() != 0) {
		fprintf(stderr, "DBus initialization failed");
		return 1;
	}

	bool preparingRefresh = false;
	struct timeval attackStart, now;

	while (1) {

		image = XGetImage(display, targetWindow,
			0, 0, win_info.width, win_info.height,
			AllPlanes, ZPixmap);

		if (image != NULL)
		{
			XImageToCvMat(image, mat);
			XDestroyImage(image);

			gettimeofday(&now, NULL);

			boost::optional<cv::Point> index = findIndexLocation(mat);
			if (index)
				std::cout << *index << std::endl;
			else
				std::cout << "n/a" << std::endl;

			active = markActiveCharacter(mat);

			if (autoControl && active != -1) {
				if (attackCommandIsDisplayed(mat) && !preparingRefresh) {
					after(&now, &attackStart, 100000);
					preparingRefresh = true;
				}

				if (preparingRefresh && DIFF(now, attackStart) >= 0) {
					sendCommand(active, mat);
					preparingRefresh = false;
				}
			}

			cv::imshow("markup", mat);
		} else {
			std::cerr << "XGetImage returns null" << std::endl;
			XCloseDisplay(display);
			return 1;
		}

		switch (cvWaitKey(10) & 0xff) {
		case 'q':
			goto end;

		case 's':
			if (autoControl) {
				std::cout << "Turn OFF auto control" << std::endl;
			} else {
				std::cout << "Turn ON auto control" << std::endl;
			}
			autoControl = !autoControl;
			break;

		case 'd':
			image = XGetImage(display, targetWindow,
			                  0, 0, win_info.width, win_info.height,
			                  AllPlanes, ZPixmap);

			if (image != NULL) {
				writeXImageToP3File(image, "dump.ppm");
				XDestroyImage(image);
			}
			break;

		}
	}

 end:

	XCloseDisplay(display);
	return 0;
}
