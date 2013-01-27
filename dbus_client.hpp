#ifndef _DBUS_CLIENT_H_
#define _DBUS_CLIENT_H_

int dbusInit();
void dbusDisconnect();

void selectNth(int id);

void attack();
void attackParty(int characterId);
void heal(int characterId);
void throwPotion(int characterId);

#endif /* _DBUS_CLIENT_H_ */
