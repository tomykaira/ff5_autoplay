OPENCV_OPTS=-lopencv_gpu -lopencv_contrib -lopencv_legacy -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_video -lopencv_highgui -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_gpu -lopencv_contrib -lopencv_legacy -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_video -lopencv_highgui -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -I/usr/include/opencv

CFLAGS=-std=c99 -g
CPPFLAGS=-g

all: capture number

capture: capture.c
	gcc $(CFLAGS) -o capture capture.c -L/usr/X11/lib -lX11 $(OPENCV_OPTS)

number: number.cpp
	g++ $(CPPFLAGS) -o number number.cpp $(OPENCV_OPTS)
