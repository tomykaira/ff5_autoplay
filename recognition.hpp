#ifndef _RECOGNITION_H_
#define _RECOGNITION_H_

#include <boost/optional.hpp>

#include <opencv2/core/core.hpp>

int attackCommandIsDisplayed(cv::Mat mat);
int markActiveCharacter(cv::Mat mat);
boost::optional<cv::Point> findIndexLocation(cv::Mat mat);

#endif /* _RECOGNITION_H_ */
