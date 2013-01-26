OPENCV_OPTS=-lopencv_gpu -lopencv_contrib -lopencv_legacy -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_video -lopencv_highgui -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_gpu -lopencv_contrib -lopencv_legacy -lopencv_objdetect -lopencv_calib3d -lopencv_features2d -lopencv_video -lopencv_highgui -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -I/usr/include/opencv

CPPFLAGS=-g -Wall

PROG=capture
SRC=capture.cpp number.cpp
OBJS=$(patsubst %.cpp,%.o,$(SRC))
DEPENDS=$(patsubst %.cpp,%.d,$(SRC))

$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(OPENCV_OPTS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY : clean depend
clean:
	-$(RM) $(PROG) $(OBJS) $(DEPENDS)

%.d: %.c
	@set -e; $(CC) -MM $(CPPFLAGS) $< \
		| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		[ -s $@ ] || rm -f $@

-include $(DEPENDS)
