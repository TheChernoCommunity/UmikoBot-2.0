#include "LevelModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

LevelModule::LevelModule()
	: Module("Level")
{
	messageXpTimer.setInterval(30 * 1000);
	messageXpTimer.start();
	QObject::connect(&messageXpTimer, &QTimer::timeout, [this]()
	{
		for (GuildId guildId : levelData.keys())
		{
			for (UserLevelData& userLevelData : levelData[guildId])
			{
				if (userLevelData.messageCount > 0)
				{
					userLevelData.currentXp += 10 + (qrand() % 6);
					userLevelData.messageCount = 0;
				}
			}
		}
	});

	generateLevels();
	
	namespace CP = CommandPermission;
#define RANK_LIST_SIGNATURE GROUP(SPACE "list")
#define RANK_ADD_SIGNATURE GROUP(SPACE "add" IDENTIFIER UNSIGNED_INTEGER)
#define RANK_REMOVE_SIGNATURE GROUP(SPACE "remove" IDENTIFIER)
#define RANK_EDIT_SIGNATURE GROUP(SPACE "edit" IDENTIFIER GROUP(SPACE "name" IDENTIFIER "|" SPACE "level" UNSIGNED_INTEGER))

	registerCommand(Commands::Top, "top" OPTIONAL(UNSIGNED_INTEGER) OPTIONAL(UNSIGNED_INTEGER), CP::User, CALLBACK(top));
	registerCommand(Commands::GiveXp, "give-xp" USER INTEGER OPTIONAL(SPACE "level" OPTIONAL("s")), CP::Moderator, CALLBACK(giveXp));
	registerCommand(Commands::TakeXp, "take-xp" USER INTEGER OPTIONAL(SPACE "level" OPTIONAL("s")), CP::Moderator, CALLBACK(takeXp));
	registerCommand(Commands::Rank, "rank" GROUP(RANK_LIST_SIGNATURE "|" RANK_ADD_SIGNATURE "|" RANK_REMOVE_SIGNATURE "|" RANK_EDIT_SIGNATURE), CP::Moderator, CALLBACK(rank));
	registerCommand(Commands::EnableXp, "enable-xp" GROUP(CHANNEL) "+", CP::Moderator, CALLBACK(enableXp));
	registerCommand(Commands::DisableXp, "disable-xp" GROUP(CHANNEL) "+", CP::Moderator, CALLBACK(disableXp));
}

void LevelModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject rankDataObject {};
	QJsonObject userDataObject {};
	QJsonObject disabledChannelsObject {};

	for (GuildId guildId : levelRanks.keys())
	{
		QJsonObject guildJson {};
		for (int id = 0; id < levelRanks[guildId].size(); id++)
		{
			QJsonObject rankJson {};
			rankJson["name"] = levelRanks[guildId][id].name;
			rankJson["minimumLevel"] = (int) levelRanks[guildId][id].minimumLevel;

			guildJson[QString::number(id)] = rankJson;
		}

		rankDataObject[QString::number(guildId)] = guildJson;
	}
	
	for (GuildId guildId : levelData.keys())
	{
		QJsonObject guildJson {};
		for (const UserLevelData& userLevelData : levelData[guildId])
		{
			QJsonObject userJson {};
			userJson["currentXp"] = userLevelData.currentXp;

			guildJson[QString::number(userLevelData.userId)] = userJson;
		}

		userDataObject[QString::number(guildId)] = guildJson;
	}

	for (ChannelId channelId : channelsWithXpDisabled.keys())
	{
		disabledChannelsObject[QString::number(channelId)] = channelsWithXpDisabled[channelId];
	}

	mainObject["rankData"] = rankDataObject;
	mainObject["userData"] = userDataObject;
	mainObject["disabledChannels"] = userDataObject;
}

void LevelModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject rankDataObject = mainObject["rankData"].toObject();
	QJsonObject userDataObject = mainObject["userData"].toObject();
	QJsonObject disabledChannelsObject = mainObject["disabledChannels"].toObject();

	for (const QString& guildIdString : rankDataObject.keys())
	{
		QJsonObject guildJson = rankDataObject[guildIdString].toObject();
		GuildId guildId = guildIdString.toULongLong();

		for (const QString& rankIdString : guildJson.keys())
		{
			QJsonObject rankJson = guildJson[rankIdString].toObject();

			levelRanks[guildId].append(LevelRank {
				rankJson["name"].toString(),
				(unsigned int) rankJson["minimumLevel"].toInt(),
			});
		}
		
		sortRanks(guildId);
	}
	
	for (const QString& guildIdString : userDataObject.keys())
	{
		QJsonObject guildJson = userDataObject[guildIdString].toObject();
		GuildId guildId = guildIdString.toULongLong();

		for (const QString& userIdString : guildJson.keys())
		{
			QJsonObject userJson = guildJson[userIdString].toObject();
			UserId userId = userIdString.toULongLong();

			levelData[guildId][userId] = UserLevelData {
				userId,	
				userJson["currentXp"].toInt(),
			};
		}
	}

	for (const QString& channelIdString : disabledChannelsObject.keys())
	{
		channelsWithXpDisabled[channelIdString.toULongLong()] = disabledChannelsObject[channelIdString].toBool();
	}
}

void LevelModule::onMessage(const Message& message, const Channel& channel)
{
	if (!channelsWithXpDisabled[channel.id()] && !message.author().bot())
	{
		getUserLevelData(channel.guildId(), message.author().id()).messageCount += 1;
	}
}

void LevelModule::onStatus(QString& output, GuildId guildId, UserId userId)
{
	QList<UserLevelData> leaderboard = getLeaderboard(guildId);

	int leaderboardPosition = 0;
	for (int i = 0; i < leaderboard.size(); i++)
	{
		if (leaderboard[i].userId == userId)
		{
			leaderboardPosition = i + 1;
		}
	}

	UserLevelData& userData = getUserLevelData(guildId, userId);
	long long int cumulativeXp = 0;
	for (int id = 0; id < levels.size(); id++)
	{
		cumulativeXp += levels[id];
		if (cumulativeXp > userData.currentXp)
		{
			break;
		}
	}
	
	int currentLevel = getCurrentLevel(guildId, userId);
	output += QString("Rank: %1 (#%2)\n").arg(getCurrentRank(guildId, userId), QString::number(leaderboardPosition));
	output += QString("Level: %1\n").arg(QString::number(currentLevel));
	output += QString("Total XP: %1\n").arg(QString::number(userData.currentXp));
	output += QString("XP until next level: %1\n").arg(QString::number(cumulativeXp - userData.currentXp));
	output += "\n";
}

UserLevelData& LevelModule::getUserLevelData(GuildId guildId, UserId userId)
{
	levelData[guildId][userId].userId = userId; // Just in case this is first initialisation
	return levelData[guildId][userId];
}

void LevelModule::generateLevels()
{
	levels.clear();
	levels.append(LEVEL_0_XP_REQUIREMENT);
	
	for (int i = 1; i < MAX_LEVEL; i++)
	{
		levels.append(levels[i - 1] * XP_REQUIREMENT_GROWTH_RATE);
	}
}

int LevelModule::getCurrentLevel(GuildId guildId, UserId userId)
{
	const UserLevelData& userLevelData = getUserLevelData(guildId, userId);
	long long int cumulativeXp = 0;
	
	for (int i = 0; i < MAX_LEVEL; i++)
	{
		cumulativeXp += levels[i];

		if (cumulativeXp > userLevelData.currentXp)
		{
			return i;
		}
	}

	return MAX_LEVEL;
}

QString LevelModule::getCurrentRank(GuildId guildId, UserId userId)
{
	if (levelRanks[guildId].size() == 0)
	{
		return "None";
	}
	
	unsigned int currentLevel = getCurrentLevel(guildId, userId);
	
	for (int i = 0; i < levelRanks[guildId].size(); i++)
	{
		if (levelRanks[guildId][i].minimumLevel >= currentLevel)
		{
			if (i == 0)
			{
				return "None";
			}

			return levelRanks[guildId][i - 1].name;
		}
	}

	return levelRanks[guildId].back().name;
}

void LevelModule::top(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	unsigned int min = 1;
	unsigned int max = 30;

	if (args.size() == 2)
	{
		max = args[1].toUInt();
	}
	else if (args.size() == 3)
	{
		min = args[1].toUInt();
		max = args[2].toUInt();
	}

	if (min == 0 || max == 0)
	{
		SEND_MESSAGE("Your arguments must be greater than 0!");
		return;
	}

	QList<UserLevelData> leaderboard = getLeaderboard(channel.guildId());
	if (min > (unsigned int) leaderboard.size())
	{
		SEND_MESSAGE("Not enough members to create the list!");
		return;
	}
	if (max > (unsigned int) leaderboard.size())
	{
		max = (unsigned int) leaderboard.size();
	}
	if (min > max)
	{
		SEND_MESSAGE("The upper bound must be greater than the lower bound!");
		return;
	}

	QString description = "";
	unsigned int numberOfDigits = QString::number(max).size();
	unsigned int rank = min;

	for (unsigned int i = min; i <= max; i++)
	{
		QString name = UmikoBot::get().getName(channel.guildId(), leaderboard[i - 1].userId);
		if (name.isEmpty())
		{
			if (max < (unsigned int) leaderboard.size())
			{
				max += 1;
			}

			continue;
		}

		description += QString("`%1`) **%2** - Level %3\n").arg(QString::number(rank).rightJustified(numberOfDigits, ' '), name,
																QString::number(getCurrentLevel(channel.guildId(), leaderboard[i - 1].userId)));
		rank += 1;
	}

	Embed embed;
	embed.setTitle(QString("XP Leaderboard (From %1 to %2)").arg(QString::number(min), QString::number(max)));
	embed.setDescription(description);
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}

void LevelModule::giveXp(const Message& message, const Channel& channel)
{
	giveTakeXpImpl(message, channel, 1);
}

void LevelModule::takeXp(const Message& message, const Channel& channel)
{
	giveTakeXpImpl(message, channel, -1);
}

void LevelModule::rank(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	QList<LevelRank>& guildRanks = levelRanks[channel.guildId()];
	sortRanks(channel.guildId());
	
	if (args[1] == "list")
	{
		if (guildRanks.size() == 0)
		{
			SEND_MESSAGE("No ranks found in this server!");
			return;
		}

		QString description = "";
		for (int id = 0; id < guildRanks.size(); id++)
		{
			description += QString("`%1`) **%2** (requires level **%3**)\n").arg(QString::number(id), guildRanks[id].name,
																				 QString::number(guildRanks[id].minimumLevel));
		}

		Embed embed;
		embed.setTitle("Guild Ranks");
		embed.setDescription(description);
		embed.setColor(qrand() % 0xffffff);
		SEND_MESSAGE(embed);
	}
	else if (args[1] == "add")
	{
		guildRanks.append(LevelRank { args[2], args[3].toUInt() });
		SEND_MESSAGE(QString("Added rank **%1** with miniumum level **%2**!").arg(guildRanks.back().name,
																				  QString::number(guildRanks.back().minimumLevel)));
		return;
	}
	else if (args[1] == "remove")
	{
		for (int id = 0; id < guildRanks.size(); id++)
		{
			if (guildRanks[id].name == args[2])
			{
				SEND_MESSAGE(QString("Removed rank **%1**!").arg(guildRanks[id].name));
				guildRanks.erase(guildRanks.begin() + id);
				return;
			}
		}

		SEND_MESSAGE("There is no rank with that name!");
	}
	else if (args[1] == "edit")
	{
		for (int id = 0; id < guildRanks.size(); id++)
		{
			if (guildRanks[id].name == args[2])
			{
				if (args[3] == "name")
				{
					SEND_MESSAGE(QString("Edited name of rank **%1** to be **%2**!").arg(guildRanks[id].name, args[4]));
					guildRanks[id].name = args[4];
				}
				else if (args[3] == "level")
				{
					guildRanks[id].minimumLevel = args[4].toUInt();
					SEND_MESSAGE(QString("Edited minimum level of rank **%1** to be **%2**!").arg(guildRanks[id].name,
																								  QString::number(guildRanks[id].minimumLevel)));
				}

				return;
			}
		}

		SEND_MESSAGE("There is no rank with that name!");
	}
}

void LevelModule::enableXp(const Message& message, const Channel& channel)
{
	enableDisableXpImpl(message, channel, true);
}

void LevelModule::disableXp(const Message& message, const Channel& channel)
{
	enableDisableXpImpl(message, channel, false);
}

void LevelModule::giveTakeXpImpl(const Message& message, const Channel& channel, int multiplier)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	UserId userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[1]);
	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}
	
	int amountToAdd = args[2].toInt() * multiplier;
	UserLevelData& userLevelData = getUserLevelData(channel.guildId(), userId);
	long long int initialXp = userLevelData.currentXp;
	int initialLevel = getCurrentLevel(channel.guildId(), userId);

	if (args.size() == 3)
	{
		// Adds XP directly
		userLevelData.currentXp += amountToAdd;

		if (userLevelData.currentXp < 0)
		{
			userLevelData.currentXp = 0;
		}
	}
	else
	{
		// Adds a number of levels
		int currentLevel = getCurrentLevel(channel.guildId(), userId);

		if (multiplier > 0)
		{
			for (int i = 1; i < amountToAdd + 1; i++)
			{
				if (currentLevel + i >= MAX_LEVEL)
				{
					break;
				}

				userLevelData.currentXp += levels[currentLevel + i];
			}
		}
		else
		{
			for (int i = 0; i > amountToAdd; i--)
			{
				if (currentLevel + i < 0)
				{
					break;
				}
				
				userLevelData.currentXp -= levels[currentLevel + i];
			}
		}
	}

	QString format = multiplier > 0 ? "Added **%1 XP (%2 levels)** to %3!" : "Removed **%1 XP (%2 levels)** from %3!";
	SEND_MESSAGE(format.arg(QString::number(abs(userLevelData.currentXp - initialXp)),
							QString::number(abs(getCurrentLevel(channel.guildId(), userId) - initialLevel)),
							UmikoBot::get().getName(channel.guildId(), userId)));
}

void LevelModule::enableDisableXpImpl(const Message& message, const Channel& channel, bool enable)
{
	UmikoBot::get().getGuildChannels(channel.guildId()).then([this, message, channel, enable](const QList<Channel>& channels)
	{
		QStringList args = message.content().split(QRegularExpression(SPACE));
		QString output = "";
		
		for (int i = 1; i < args.size(); i++)
		{
			ChannelId channelId = UmikoBot::get().getChannelIdFromArgument(channels, args[i]);
			if (channelId)
			{
				channelsWithXpDisabled[channelId] = !enable;
				output += QString("%1 XP in channel <#%2>").arg(enable ? "Enabled" : "Disabled", QString::number(channelId));
			}
			else
			{
				output += QString("Could not find channel '%1'!").arg(args[i]);
			}
		}

		SEND_MESSAGE(output);
	});
}

void LevelModule::sortRanks(GuildId guildId)
{
	QList<LevelRank>& ranks = levelRanks[guildId];
	qSort(ranks.begin(), ranks.end(), [](const LevelRank& first, const LevelRank& second)
	{
		return first.minimumLevel < second.minimumLevel;
	});
}

QList<UserLevelData> LevelModule::getLeaderboard(GuildId guildId)
{
	QList<UserLevelData> leaderboard = levelData[guildId].values();
	qSort(leaderboard.begin(), leaderboard.end(), [](const UserLevelData& first, const UserLevelData& second)
	{
		return first.currentXp > second.currentXp;
	});

	return leaderboard;
}
