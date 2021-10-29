#include "modules/UserModule.h"
#include "core/Permissions.h"
#include "UmikoBot.h"

using namespace Discord;

UserModule::UserModule()
	: Module("User")
{
	namespace CP = CommandPermission;

	registerCommand(Commands::SetTimezone, "set-timezone" SPACE OPTIONAL("UTC") OPTIONAL("[+-]") "[0-9]{1,2}" OPTIONAL(":[0-9]{1,2}"), CP::User, CALLBACK(setTimezone));
}

void UserModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject timeDataObject {};
	for (UserId userId : userData.keys())
	{
		timeDataObject[QString::number(userId)] = userData[userId];
	}

	mainObject["timezoneData"] = timeDataObject;
}

void UserModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject timeDataObject = mainObject["timezoneData"].toObject();
	for (const QString& userIdString : timeDataObject.keys())
	{
		userData[userIdString.toULongLong()] = timeDataObject[userIdString].toInt();
	}
}

void UserModule::onStatus(QString& output, GuildId guildId, UserId userId)
{
	int userSecondsOffset = userData[userId];
	
	output += QString("Time offset: %1\n").arg(getUserTimezoneString(userId));
	output += QString("Current time: %1\n").arg(QDateTime::currentDateTimeUtc().addSecs(userSecondsOffset).toString("hh':'mm"));
	output += "\n";
}

QString UserModule::getUserTimezoneString(UserId userId)
{
	int userSecondsOffset = userData[userId];
	QString sign = userSecondsOffset < 0 ? "-" : "+";
	int hours = abs(userSecondsOffset) / 3600;
	int minutes = (abs(userSecondsOffset) % 3600) / 60;

	return QString("UTC%1%2:%3").arg(sign).arg(QString::number(hours), 2, '0').arg(QString::number(minutes), 2, '0');
}

void UserModule::setTimezone(const Message& message, const Channel& channel)
{
	// Removes UTC prefix if it exists
	QString offset = message.content().split(QRegularExpression(SPACE))[1];
	if (offset.startsWith("UTC"))
	{
		offset = offset.mid(3);
	}

	// Parses the sign
	int multiplier = 1;
	if (offset.startsWith("-"))
	{
		multiplier = -1;
	}
	if (offset.startsWith("+") || offset.startsWith("-"))
	{
		offset = offset.mid(1);
	}

	// Splits the offset into its two parts
	QStringList parts = offset.split(":");
	int hours = parts[0].toInt();
	int minutes = 0;

	if (parts.size() > 1)
	{
		minutes = parts[1].toInt();
	}

	// Bounds checking for offset parts
	auto clamp = [](int value, int min, int max) { return (value < min) ? min : ((value > max) ? max : value); };
	hours = clamp(hours, -12, 14);
	minutes = clamp(minutes, 0, 59);

	userData[message.author().id()] = multiplier * ((hours * 3600) + (minutes * 60));
	SEND_MESSAGE(QString("Set timezone to **%1**!").arg(getUserTimezoneString(message.author().id())));
}
