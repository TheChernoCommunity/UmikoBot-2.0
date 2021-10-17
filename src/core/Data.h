#pragma once

#include <QtCore/QList>
#include <Discord/Client.h>

#include "core/Core.h"

struct UserData
{
	QString username;
	QString nickname;
};

struct GuildData
{
public:
	QJsonObject writeToObject() const;
	static GuildData createFromObject(GuildId guildId, const QJsonObject& object);
	
public:
	GuildId guildId = 0;
	UserId ownerId = 0;
	QList<Discord::Role> roles {};
	
	QString prefix = "!";
	
	QMap<UserId, UserData> userData {};
};
