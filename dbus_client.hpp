#ifndef _DBUS_CLIENT_H_
#define _DBUS_CLIENT_H_

#include <opencv2/core/core.hpp>

int dbusInit();
void dbusDisconnect();

void selectNth(cv::Mat * rawImage, int id);

void attack(cv::Mat * rawImage);
void attackParty(cv::Mat * rawImage, int characterId);
void heal(cv::Mat * rawImage, int characterId);
void throwPotion(cv::Mat * rawImage, int characterId);

#endif /* _DBUS_CLIENT_H_ */
