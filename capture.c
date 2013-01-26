/**
 * gcc capture_screen_x11-01.c -L/usr/X11/lib -lX11
 * http://d.hatena.ne.jp/sa-y/20070306
**/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "cv.h"
#include "highgui.h"

/* dst is initialized with cvCreateImage(cvSize(src.width, src.height), IPL_DEPTH_8U, 3); */
void XImageToIplImage(const XImage *src, IplImage *dst)
{
	for (int y = 0;y < src->height;y++) {
		char * cp = src->data + y * src->bytes_per_line;
		for (int x = 0;x < src->width;x++) {
			uint32_t pixel = *(uint32_t *)cp;
			((uchar*)(dst->imageData + dst->widthStep*y))[x*3  ] = (pixel & 0x000000ff) >> 0;
			((uchar*)(dst->imageData + dst->widthStep*y))[x*3+1] = (pixel & 0x0000ff00) >> 8;
			((uchar*)(dst->imageData + dst->widthStep*y))[x*3+2] = (pixel & 0x00ff0000) >> 16;
		}
	}
}

Window Window_With_Name(
    Display *dpy,
    Window top,
    const char *name)
{
	Window *children, dummy;
	unsigned int nchildren;
	int i;
	Window w=0;
	char *window_name;

	if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
	  return(top);

	if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
	  return(0);

	for (i=0; i<nchildren; i++) {
		w = Window_With_Name(dpy, children[i], name);
		if (w)
		  break;
	}
	if (children) XFree ((char *)children);
	return(w);
}

int main(int argc, char* argv[])
{
	Display* display;
	int screen;
	Window rootWindow, targetWindow;
	XWindowAttributes win_info;
	int width, height;
	XImage *image;

IplImage * outputImage;

	if (argc < 2) {
		printf("Usage: %s WINDOW_NAME\n", argv[0]);
		exit(1);
	}

	display = XOpenDisplay("");
	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);

	targetWindow = Window_With_Name(display, rootWindow, argv[1]);

	XGetWindowAttributes(display, targetWindow, &win_info);
	width = win_info.width;
	height = win_info.height;

	// while (cvWaitKey(10) & 0xff != 0x1b) {

		image = XGetImage(display, targetWindow,
			0, 0, win_info.width/2, win_info.height/2,
			AllPlanes, ZPixmap);

		if (image != NULL)
		{
			if (image->bits_per_pixel == 32) {
				outputImage = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, 3);
				XImageToIplImage(image, outputImage);
				cvShowImage("capture", outputImage);
			} else {
				fprintf(stderr, "Not Supported format : bits_per_pixel = %d\n", image->bits_per_pixel);
				return(1);
			}
			XFree(image);
		}
	// }

	cvWaitKey(0);

	XCloseDisplay(display);
	return 0;
}
