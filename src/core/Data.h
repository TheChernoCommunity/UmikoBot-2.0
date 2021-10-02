#pragma once

#include <QtCore/QList>
#include <Discord/Client.h>

struct UserData
{
	QString username;
	QString nickname;
};

struct GuildData
{
public:
	QJsonObject writeToObject() const;
	static GuildData createFromObject(snowflake_t guildId, const QJsonObject& object);
	
public:
	snowflake_t guildId;
	snowflake_t ownerId;
	QList<Discord::Role> roles;
	
	QString prefix = "!";
	
	QMap<snowflake_t /* userId */, UserData> userData;
};
