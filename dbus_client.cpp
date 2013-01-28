#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <dbus/dbus.h>

#include "dbus_client.hpp"
#include "recognition.hpp"

static DBusConnection* conn;

int dbusInit()
{
	DBusError err;
	int ret;
	// initialise the errors
	dbus_error_init(&err);

	// connect to the bus
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		return 1;
	}

	// request a name on the bus
	ret = dbus_bus_request_name(conn, "com.snes9x.autoPlayer",
	                            DBUS_NAME_FLAG_REPLACE_EXISTING,
	                            &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		return 1;
	}

	return 0;
}

void dbusDisconnect()
{
	dbus_connection_close(conn);
}

void dbusCallMethod(bool isButton, const char * methodName)
{
	DBusMessage* msg;
	DBusPendingCall* pending;
	const char * interface = isButton ? "com.snes9x.emulator.Joypad" : "com.snes9x.emulator";

	msg = dbus_message_new_method_call("com.snes9x.emulator",
	                                   "/",
	                                   interface,
	                                   methodName);
	if (NULL == msg) {
		fprintf(stderr, "Message Null\n");
		return;
	}

	// send message and get a handle for a reply
	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) { // -1 is default timeout
		fprintf(stderr, "Out Of Memory!\n");
		return;
	}
	if (NULL == pending) {
		fprintf(stderr, "Pending Call Null\n");
		return;
	}
	dbus_connection_flush(conn);

	// free message
	dbus_message_unref(msg);
}

void pressButton(cv::Mat * rawImage, std::string button)
{
	boost::optional<cv::Point> indexPosition;
	while ((indexPosition = findIndexLocation(*rawImage)) == NULL)
		usleep(1000); // until index is found

	dbusCallMethod(true, button.c_str());
	while (findIndexLocation(*rawImage) == indexPosition)
		usleep(1000); // until index move
}

void selectNthCaracter(cv::Mat * rawImage, int characterId)
{
	if (characterId < 3) {
		for (int i = 0; i < characterId; ++i) {
			pressButton(rawImage, "Down");
		}
	} else {
		pressButton(rawImage, "Up");
	}
}

void attack(cv::Mat * rawImage)
{
	std::cout << "たたかう" << std::endl;
	pressButton(rawImage, "A");
	pressButton(rawImage, "A");
}

void attackParty(cv::Mat * rawImage, int characterId)
{
	std::cout << "たたかう to " << characterId << std::endl;
	pressButton(rawImage, "A");
	pressButton(rawImage, "Right");
	selectNthCaracter(rawImage, characterId);
	pressButton(rawImage, "A");
}

void heal(cv::Mat * rawImage, int characterId)
{
	std::cout << "ケアル to " << characterId << std::endl;
	pressButton(rawImage, "Down");
	pressButton(rawImage, "Down");
	pressButton(rawImage, "A");
	pressButton(rawImage, "A");
	selectNthCaracter(rawImage, characterId);
	pressButton(rawImage, "A");
}

void throwPotion(cv::Mat * rawImage, int characterId)
{
	std::cout << "ポーション to " << characterId << std::endl;
	pressButton(rawImage, "Up");
	pressButton(rawImage, "A");
	pressButton(rawImage, "A");
	pressButton(rawImage, "A");
	selectNthCaracter(rawImage, characterId);
	pressButton(rawImage, "A");
}

void selectNth(cv::Mat * rawImage, int id)
{
	for (int i = 0; i < id; ++i) {
		pressButton(rawImage, "Down");
	}
	pressButton(rawImage, "A");
}
