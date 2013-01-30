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

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "xlib_ext.hpp"
#include "number.hpp"
#include "dbus_client.hpp"
#include "recognition.hpp"
#include "time_util.hpp"
#include "dungeon.hpp"
#include "dungeon/Map.hpp"

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
		command(rawImage, std::vector<int> { 1, 0 });
		break;
	case 1:
		std::cout << "レナ" << std::endl;
		attack(rawImage);
		// guard(rawImage);
		break;
	case 2:
		std::cout << "クルル" << std::endl;
		command(rawImage, std::vector<int> { 1, 0 });
		break;
	case 3:
		std::cout << "バッツ" << std::endl;
		command(rawImage, std::vector<int> { 2, 0 });
		break;
	}

	return 0;
}

int main(int argc, char* argv[])
{

	cv::Mat rawImage, mat;

	bool live = true;

	if (dbusInit() != 0) {
		fprintf(stderr, "DBus initialization failed");
		return 1;
	}

	boost::thread updateMatrixThread = boost::thread(updateGameMatrix, &live, &rawImage);

	dungeon::Map * map = NULL;

	while (1) {

		if (rawImage.empty())
			continue;

		mat = rawImage.clone();

		if (map && inField(mat)) {
			map->detectSymbols(mat);
		}

		cv::imshow("markup", mat);

		switch (cvWaitKey(10) & 0xff) {
		case 'q':
			goto end;

		case 'h':
			map->move(LEFT);
			break;
		case 'j':
			map->move(DOWN);
			break;
		case 'k':
			map->move(UP);
			break;
		case 'l':
			map->move(RIGHT);
			break;

		case 'd':
			cv::imwrite("dump.bmp", rawImage);
			break;

		case 's':
			map->debug();
			break;

		case 'r':
			if (map) {
				delete map;
				map = NULL;
			}
			map = new dungeon::Map(cropGroundTemplate(mat));
			break;
		}
	}

 end:

	delete map;

	live = false;
	updateMatrixThread.join();
	return 0;
}
