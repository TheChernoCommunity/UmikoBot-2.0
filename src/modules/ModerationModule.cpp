#include "ModerationModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <QJsonArray>

using namespace Discord;

ModerationModule::ModerationModule()
	: Module("Moderation")
{
	namespace CP = CommandPermission;
	registerCommand(Commands::ModerateInvitations, "moderate-invitations" OPTIONAL(SPACE "(on|off)"), CP::Moderator, CALLBACK(moderateInvitations));
	registerCommand(Commands::Warn, "warn" USER MULTI_LINE_TEXT, CP::Moderator, CALLBACK(warn));
	registerCommand(Commands::Warnings, "warnings" SPACE "(list" USER "|list-all" USER "|remove" USER UNSIGNED_INTEGER ")", CP::Moderator, CALLBACK(warnings));
}

void ModerationModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject userWarningsObject {};
	for (UserId userId : userWarnings.keys())
	{
		QJsonArray userWarningsArray;
		for (const UserWarning& warning : userWarnings[userId])
		{
			QJsonObject warningJson {};
			warningJson["warnedBy"] = QString::number(warning.warnedBy);
			warningJson["when"] = warning.when.toString();
			warningJson["message"] = warning.message;
			warningJson["isActive"] = warning.isActive;

			userWarningsArray.append(warningJson);
		}

		userWarningsObject[QString::number(userId)] = userWarningsArray;
	}

	mainObject["userWarnings"] = userWarningsObject;
}

void ModerationModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject userWarningsObject = mainObject["userWarnings"].toObject();
	for (const QString& userIdString : userWarningsObject.keys())
	{
		QJsonArray userWarningsArray = userWarningsObject[userIdString].toArray();
		for (const QJsonValue& warningJson : userWarningsArray)
		{
			QJsonObject warningObject = warningJson.toObject();
			userWarnings[userIdString.toULongLong()].append(UserWarning {
				warningJson["warnedBy"].toString().toULongLong(),
				QDateTime::fromString(warningJson["when"].toString()),
				warningJson["message"].toString(),
				warningJson["isActive"].toBool(),
			});
		}
	}
}

void ModerationModule::onMessage(const Message& message, const Channel& channel)
{
	if (isModeratingInvitations)
	{
		if (message.content().contains("discord.gg/", Qt::CaseInsensitive))
		{
			::Permissions::contains(channel.guildId(), message.author().id(), CommandPermission::Moderator, [this, message](bool result)
			{
				if (result)
				{
					// Moderators are excluded from this
					return;
				}

				UmikoBot::get().deleteMessage(message.channelId(), message.id());
				UmikoBot::get().createDm(message.author().id()).then([this, message](const Channel& channel)
				{
					UmikoBot::get().createMessage(channel.id(), "Invitation links to discord servers are not allowed. Please take it to DMs!\n"
												  "Here is the message which you posted:");
					UmikoBot::get().createMessage(channel.id(), message.content());
				});
			});
		}
	}
}

void ModerationModule::moderateInvitations(const Message& message, const Channel& channel)
{
	(void) channel;
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() == 1)
	{
		SEND_MESSAGE(QString("Moderating invitations is turned %1!").arg(isModeratingInvitations ? "on" : "off"));
		return;
	}
	
	if (args[1] == "on")
	{
		isModeratingInvitations = true;
	}
	else if (args[1] == "off")
	{
		isModeratingInvitations = false;
	}

	SEND_MESSAGE(QString("Moderating invitations has been turned %1!").arg(args[1]));
}

void ModerationModule::warn(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	
	UserId userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[1]);
	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}

	// Removes everything but the message argument
	QString msg = message.content();
	msg.remove(0, args[0].size());
	msg.remove(0, msg.indexOf(QRegularExpression("\\S")));
	msg.remove(0, args[1].size());
	msg.remove(0, msg.indexOf(QRegularExpression("\\S")));

	userWarnings[userId].append(UserWarning { message.author().id(), msg });
	unsigned int numberOfWarnings = countWarningsForUser(userId);
	QString numberOfWarningsString = "";

	switch (numberOfWarnings)
	{
	case 1: numberOfWarningsString = "First"; break;
	case 2: numberOfWarningsString = "Second"; break;
	case 3: numberOfWarningsString = "Third"; break;

	// Good enough, shouldn't be seen
	default: numberOfWarningsString = QString("%1th").arg(QString::number(numberOfWarnings));
	}

	SEND_MESSAGE(QString("%1 warning for <@%2>").arg(numberOfWarningsString, QString::number(userId)));
}

void ModerationModule::warnings(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	UserId userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[2]);
	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}
	
	if (args[1] == "remove")
	{
		int id = userWarnings[userId].size() - args[3].toInt() - 1;
		if (id < 0 || id >= userWarnings[userId].size())
		{
			SEND_MESSAGE(QString("That is an invalid ID! Use `%1warnings list` to see IDs!").arg(UmikoBot::get().getGuildData()[channel.guildId()].prefix));
			return;
		}

		UserWarning warning = userWarnings[userId].takeAt(id);
		SEND_MESSAGE(QString("Removed warning by %1:\n**%2**").arg(UmikoBot::get().getName(channel.guildId(), warning.warnedBy), warning.message));
	}
	else if (args[1] == "list" || args[1] == "list-all")
	{
		bool activeWarningsOnly = args[1] == "list-all" ? false : true;
		unsigned int numberOfWarnings = countWarningsForUser(userId, activeWarningsOnly);
		if (numberOfWarnings == 0)
		{
			SEND_MESSAGE("Nothing to see here...");
			return;
		}

		UmikoBot::get().getAvatar(channel.guildId(), userId).then(
			[this, message, channel, args, userId, activeWarningsOnly, numberOfWarnings](const QString& icon)
		{
			qSort(userWarnings[userId].begin(), userWarnings[userId].end(), [](const UserWarning& first, const UserWarning& second)
			{
				return first.when > second.when;
			});

			QString description = "";
			bool hasOutputInactiveMessage = false;
			
			for (int id = 0; id < userWarnings[userId].size(); id++)
			{
				const UserWarning& warning = userWarnings[userId][id];
				if (!warning.isActive)
				{
					if (activeWarningsOnly)
					{
						continue;
					}

					if (!hasOutputInactiveMessage)
					{
						description += "\n===== Expired ======\n\n";
						hasOutputInactiveMessage = true;
					}
				}

				description += QString("%1 - %2 warned on %3\n**%4**\n\n").arg(QString::number(userWarnings[userId].size() - id - 1),
																			   UmikoBot::get().getName(channel.guildId(), warning.warnedBy),
																			   warning.when.toString("yyyy-MM-dd hh:mm:ss"),
																			   warning.message);
			}

			Embed embed;
			embed.setColor(qrand() % 0xffffff);
			embed.setAuthor(EmbedAuthor { QString("Warnings for %1").arg(UmikoBot::get().getName(channel.guildId(), userId)), "", icon });
			embed.setDescription(description);
			SEND_MESSAGE(embed);
		});
	}
}

unsigned int ModerationModule::countWarningsForUser(UserId userId, bool activeWarningsOnly)
{
	if (!activeWarningsOnly)
	{
		return userWarnings[userId].size();
	}

	unsigned int total = 0;
	for (const UserWarning& warning : userWarnings[userId])
	{
		if (warning.isActive)
		{
			total += 1;
		}
	}

	return total;
}
