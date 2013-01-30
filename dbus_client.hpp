#ifndef _DBUS_CLIENT_H_
#define _DBUS_CLIENT_H_

#include <vector>

#include <opencv2/core/core.hpp>

int dbusInit();
void dbusDisconnect();
void dbusCallMethod(bool isButton, const char * methodName);

void selectNth(cv::Mat * rawImage, int id);

void attack(cv::Mat * rawImage);
void attackParty(cv::Mat * rawImage, int characterId);
void command(cv::Mat * rawImage, std::vector<int> ids);
void guard(cv::Mat * rawImage);

#endif /* _DBUS_CLIENT_H_ */
