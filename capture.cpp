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
#include <sys/time.h>

#define DIFF(end, start) (((end).tv_sec - (start).tv_sec)*1000*1000 + (end).tv_usec - (start).tv_usec)

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "number.hpp"
#include "dbus_client.hpp"

void after(const struct timeval * from, struct timeval * to, int diff)
{
	int new_usec = from->tv_usec + diff;
	to->tv_usec = new_usec % (1000*1000);
	to->tv_sec  = from->tv_sec + new_usec / (1000*1000);
}

/* dst is initialized with cvCreateImage(cvSize(src.width, src.height), IPL_DEPTH_8U, 3); */
static IplImage * XImageToIplImage(const XImage *src)
{
	int x, y;
	char *cp;
	uint32_t pixel;

	IplImage * dst = cvCreateImage(cvSize(src->width, src->height), IPL_DEPTH_8U, 3);

	for (y = 0;y < src->height;y++) {
		cp = src->data + y * src->bytes_per_line;
		for (x = 0;x < src->width;x++) {
			pixel = *(uint32_t *)cp;

			((uchar*)(dst->imageData + dst->widthStep*y))[x*3  ] = (pixel & 0x000000ff) >> 0;
			((uchar*)(dst->imageData + dst->widthStep*y))[x*3+1] = (pixel & 0x0000ff00) >> 8;
			((uchar*)(dst->imageData + dst->widthStep*y))[x*3+2] = (pixel & 0x00ff0000) >> 16;
			cp += 4;
		}
	}

	return dst;
}

static
void writeXImageToP3File(XImage *image, const char *file_path)
{
	FILE *file;

	file = fopen(file_path, "wb");
	if (file != NULL)
	{
		int x, y;
		char *cp;
		uint32_t pixel;

		fprintf(file, "P3\n");
		fprintf(file, "%d %d\n", image->width, image->height);
		fprintf(file, "255\n");
		for (y = 0;y < image->height;y++)
		{
			cp = image->data + y * image->bytes_per_line;
			for (x = 0;x < image->width;x++)
			{
				pixel = *(uint32_t *)cp;
				fprintf(file, "%d %d %d ",
					(pixel & 0x00ff0000) >> 16,
					(pixel & 0x0000ff00) >> 8,
					(pixel & 0x000000ff) >> 0);
				cp += 4;
			}
			fprintf(file, "\n");
		}
		fclose(file);
	}
}

Window windowWithName(
    Display *dpy,
    Window top,
    const char *name)
{
	Window *children, dummy;
	unsigned int nchildren;
	Window w=0;
	char *window_name;

	if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
	  return(top);

	if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
	  return(0);

	for (unsigned int i = 0; i<nchildren; i++) {
		w = windowWithName(dpy, children[i], name);
		if (w)
		  break;
	}
	if (children) XFree ((char *)children);
	return(w);
}

// cv::Rect(16, 0, 480, 448)    : full play area
// cv::Rect(16, 320, 480, 120)  : command area (outer bound)
// cv::Rect(16, 320, 96, 120)   : enemy name
// cv::Rect(112, 320, 112, 120) : command area
// cv::Rect(224, 320, 272, 120) : character status
// cv::Rect(332, 320, 72, 120)  : hp area

cv::Mat templateImage = cv::imread("templates/attack.bmp", 1);

int attackCommandIsDisplayed(cv::Mat mat)
{
	cv::Mat commandArea = mat(cv::Rect(112, 320, 112, 120));
	cv::Mat result;

	cv::matchTemplate(commandArea, templateImage, result, CV_TM_CCOEFF_NORMED);

	double maxScore;
	cv::Point point;
	cv::Rect roi(0, 0, templateImage.cols, templateImage.rows);
	cv::minMaxLoc(result, NULL, &maxScore, NULL, &point);
	roi.x = point.x;
	roi.y = point.y;

	cv::rectangle(commandArea, roi, cv::Scalar(0, 0, 255), 3);

	result.release();

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

	IplImage * outputImage;

	int active;
	cv::Mat mat;

	const char * ffWindowName = "\"FINAL FANTASY 5\" Snes9x: Linux: 1.53";

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

	while ((cvWaitKey(10) & 0xff) != 'q') {

		image = XGetImage(display, targetWindow,
			0, 0, win_info.width, win_info.height,
			AllPlanes, ZPixmap);

		if (image != NULL)
		{
			if (image->bits_per_pixel == 32) {
				outputImage = XImageToIplImage(image);

				mat = cv::cvarrToMat(outputImage);

				gettimeofday(&now, NULL);

				active = markActiveCharacter(mat);

				if (active != -1) {
					std::cout << "Active " << active << std::endl;
					if (attackCommandIsDisplayed(mat) && !preparingRefresh) {
						std::cout << "Preparing" << std::endl;

						after(&now, &attackStart, 100000);
						preparingRefresh = true;
					}

					if (preparingRefresh && DIFF(now, attackStart) >= 0) {
						std::cout << "executing" << std::endl;

						sendCommand(active, mat);
						preparingRefresh = false;
					}
				}

				cv::imshow("markup", mat);

				cvReleaseImage(&outputImage);
			} else {
				fprintf(stderr, "Not Supported format : bits_per_pixel = %d\n", image->bits_per_pixel);
				return(1);
			}
			XFree(image);
		}
	}

	XCloseDisplay(display);
	return 0;
}
