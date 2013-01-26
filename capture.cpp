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
	case 0: // ファリス
	case 3: // バッツ
		std::cout << "Attack" << std::endl;
		attack();
		break;

	case 1: // レナ
		std::cout << "レナ" << std::endl;
		// if (lowestHP < 150) {
			std::cout << "\tHeal " << lowestHPCharacter << std::endl;
			heal(lowestHPCharacter);
		// } else {
		// 	std::cout << "\tAttack" << std::endl;
		// 	attack();
		// }
		break;

	case 2: // ガラフ
		std::cout << "ガラフ" << std::endl;
		// if (lowestHP < 50) {
			std::cout << "\tPotion " << lowestHPCharacter << std::endl;
			throwPotion(lowestHPCharacter);
		// } else {
		// 	std::cout << "\tAttack" << std::endl;
		// 	attack();
		// }
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

	struct timeval previousCommandTime, now;
	gettimeofday(&previousCommandTime, NULL); // initialize

	if (argc < 2) {
		printf("Usage: %s WINDOW_NAME\n", argv[0]);
		exit(1);
	}

	display = XOpenDisplay("");
	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);

	targetWindow = windowWithName(display, rootWindow, argv[1]);

	XGetWindowAttributes(display, targetWindow, &win_info);

	if (dbusInit() != 0) {
		fprintf(stderr, "DBus initialization failed");
		return 1;
	}

	while ((cvWaitKey(10) & 0xff) != 'q') {

		image = XGetImage(display, targetWindow,
			0, 0, win_info.width, win_info.height,
			AllPlanes, ZPixmap);

		if (image != NULL)
		{
			if (image->bits_per_pixel == 32) {
				outputImage = XImageToIplImage(image);

				cvShowImage("capture", outputImage);

				cv::Mat mat = cv::cvarrToMat(outputImage);

				gettimeofday(&now, NULL);
				if ((active = markActiveCharacter(mat)) != -1
				    && DIFF(now, previousCommandTime) > 100*1000) {
					if (sendCommand(active, mat) == 0) {
						gettimeofday(&previousCommandTime, NULL);
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
