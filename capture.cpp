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
