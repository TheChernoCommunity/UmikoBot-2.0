#include "Data.h"

#include <QJsonObject>

QJsonObject GuildData::writeToObject() const
{
	QJsonObject result;
	result["prefix"] = prefix;

	return result;
}

GuildData GuildData::createFromObject(snowflake_t guildId, const QJsonObject& object)
{
	GuildData result {};
	result.guildId = guildId;

	result.prefix = object["prefix"].toString("!");
	if (result.prefix == "")
	{
		result.prefix = "!";
	}

	return result;
}
