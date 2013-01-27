#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <dbus/dbus.h>

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

void dbusSendButton(const char * button)
{
	DBusMessage* msg;
	DBusPendingCall* pending;

	msg = dbus_message_new_method_call("com.snes9x.emulator", // target for the method call
	                                   "/", // object to call on
	                                   "com.snes9x.emulator.Joypad", // interface to call on
	                                   button); // method name
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

	usleep(1000*1000);
}

void selectNthCaracter(int characterId)
{
	if (characterId < 3) {
		for (int i = 0; i < characterId; ++i) {
			dbusSendButton("Down");
		}
	} else {
		dbusSendButton("Up");
	}
}

void attack()
{
	std::cout << "たたかう" << std::endl;
	dbusSendButton("A");
	dbusSendButton("A");
}

void attackParty(int characterId)
{
	std::cout << "たたかう to " << characterId << std::endl;
	dbusSendButton("A");
	dbusSendButton("Right");
	selectNthCaracter(characterId);
	dbusSendButton("A");
}

void heal(int characterId)
{
	std::cout << "ケアル to " << characterId << std::endl;
	dbusSendButton("Down");
	dbusSendButton("Down");
	dbusSendButton("A");
	dbusSendButton("A");
	selectNthCaracter(characterId);
	dbusSendButton("A");
}

void throwPotion(int characterId)
{
	std::cout << "ポーション to " << characterId << std::endl;
	dbusSendButton("Up");
	dbusSendButton("A");
	dbusSendButton("A");
	dbusSendButton("A");
	selectNthCaracter(characterId);
	dbusSendButton("A");
}

void selectNth(int id)
{
	for (int i = 0; i < id; ++i) {
		dbusSendButton("Down");
	}
	dbusSendButton("A");
}
