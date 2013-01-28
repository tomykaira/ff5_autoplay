#include <iostream>
#include <stdio.h>
#include <X11/Xutil.h>
#include "xlib_ext.hpp"

/* dst is initialized with cvCreateImage(cvSize(src.width, src.height), IPL_DEPTH_8U, 3); */
void XImageToCvMat(const XImage *src, cv::Mat& result)
{
	int x, y;
	char *cp;
	uint32_t pixel;

	result.create(cv::Size(src->width, src->height), CV_8UC3);

	for (y = 0;y < src->height;y++) {
		cp = src->data + y * src->bytes_per_line;
		for (x = 0;x < src->width;x++) {
			pixel = *(uint32_t *)cp;

			result.data[(y*result.cols + x)*3] = (pixel & 0x000000ff) >> 0;
			result.data[(y*result.cols + x)*3 + 1] = (pixel & 0x0000ff00) >> 8;
			result.data[(y*result.cols + x)*3 + 2] = (pixel & 0x00ff0000) >> 16;
			cp += 4;
		}
	}
}

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

// update content of matrix
void updateGameMatrix(const bool * live, cv::Mat * mat)
{
	const char * ffWindowName = "\"FINAL FANTASY 5\" Snes9x: Linux: 1.53";

	Display* display;
	int screen;
	Window rootWindow, targetWindow;
	XWindowAttributes win_info;
	XImage *image;

	display = XOpenDisplay("");
	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);

	targetWindow = windowWithName(display, rootWindow, ffWindowName);

	XGetWindowAttributes(display, targetWindow, &win_info);

	while (*live) {
		image = XGetImage(display, targetWindow,
			0, 0, win_info.width, win_info.height,
			AllPlanes, ZPixmap);

		if (image != NULL)
		{
			XImageToCvMat(image, *mat);
			XDestroyImage(image);
		} else {
			std::cerr << "XGetImage returns null" << std::endl;
			XCloseDisplay(display);
			return;
		}
	}
	XCloseDisplay(display);
}
