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

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "xlib_ext.hpp"
#include "number.hpp"
#include "dbus_client.hpp"
#include "recognition.hpp"

#define DIFF(end, start) (((end).tv_sec - (start).tv_sec)*1000*1000 + (end).tv_usec - (start).tv_usec)

void after(const struct timeval * from, struct timeval * to, int diff)
{
	int new_usec = from->tv_usec + diff;
	to->tv_usec = new_usec % (1000*1000);
	to->tv_sec  = from->tv_sec + new_usec / (1000*1000);
}

int sendCommand(int activeCharacter, cv::Mat mat, cv::Mat * rawImage)
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
		boost::thread(attack, rawImage);
		break;
	case 1: // レナ
		std::cout << "レナ" << std::endl;
		boost::thread(attackParty, rawImage, lowestHPCharacter);
		break;
	case 2: // ガラフ
		std::cout << "ガラフ" << std::endl;
		boost::thread(attack, rawImage);
		break;
	case 3: // バッツ
		std::cout << "バッツ" << std::endl;
		boost::thread(attack, rawImage);
		break;
	}

	return 0;
}

int main(int argc, char* argv[])
{

	cv::Mat rawImage, mat;

	bool live = true;

	int active;
	bool autoControl = true;


	if (dbusInit() != 0) {
		fprintf(stderr, "DBus initialization failed");
		return 1;
	}

	bool preparingRefresh = false;
	struct timeval attackStart, now;

	boost::thread updateMatrixThread = boost::thread(updateGameMatrix, &live, &rawImage);

	while (1) {

		if (rawImage.empty())
			continue;

		mat = rawImage.clone();

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
				sendCommand(active, mat, &rawImage);
				preparingRefresh = false;
			}
		}

		cv::imshow("markup", mat);

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
			cv::imwrite("dump.bmp", rawImage);
			break;

		}
	}

 end:

	live = false;
	updateMatrixThread.join();
	return 0;
}
