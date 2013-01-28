#ifndef _XLIB_EXT_H_
#define _XLIB_EXT_H_

#include <X11/Xlib.h>
#include <opencv2/core/core.hpp>

Window windowWithName(Display *dpy, Window top, const char *name);

void writeXImageToP3File(XImage *image, const char *file_path);

void XImageToCvMat(const XImage *src, cv::Mat& result);

void updateGameMatrix(const bool * live, cv::Mat * mat);

#endif /* _XLIB_EXT_H_ */
