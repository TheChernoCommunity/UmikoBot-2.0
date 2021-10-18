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
		for (QList<UserLevelData>& guildLevelData : levelData)
		{
			for (UserLevelData& userLevelData : guildLevelData)
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
#define RANK_ADD_SIGNATURE GROUP(SPACE "add" UNSIGNED_INTEGER IDENTIFIER)
#define RANK_REMOVE_SIGNATURE GROUP(SPACE "remove" UNSIGNED_INTEGER)
#define RANK_EDIT_SIGNATURE GROUP(SPACE "edit" UNSIGNED_INTEGER GROUP(SPACE "name" IDENTIFIER "|" SPACE "level" UNSIGNED_INTEGER))

	registerCommand(Commands::Top, "top" OPTIONAL(UNSIGNED_INTEGER) OPTIONAL(UNSIGNED_INTEGER), CP::User, CALLBACK(top));
	registerCommand(Commands::GiveXp, "give-xp" USER INTEGER OPTIONAL(SPACE "level" OPTIONAL("s")), CP::Moderator, CALLBACK(giveXp));
	registerCommand(Commands::TakeXp, "take-xp" USER INTEGER OPTIONAL(SPACE "level" OPTIONAL("s")), CP::Moderator, CALLBACK(takeXp));
	registerCommand(Commands::Rank, "rank" GROUP(RANK_LIST_SIGNATURE "|" RANK_ADD_SIGNATURE "|" RANK_REMOVE_SIGNATURE "|" RANK_EDIT_SIGNATURE), CP::Moderator, CALLBACK(rank));
}

LevelModule::~LevelModule()
{
}

void LevelModule::onSave(QJsonObject& mainObject) const
{
}

void LevelModule::onLoad(const QJsonObject& mainObject)
{
}

void LevelModule::onMessage(const Message& message, const Channel& channel)
{
	getUserLevelData(channel.guildId(), message.author().id()).messageCount += 1;
}

UserLevelData& LevelModule::getUserLevelData(GuildId guildId, UserId userId)
{
	for (UserLevelData& userLevelData : levelData[guildId])
	{
		if (userLevelData.userId == userId)
		{
			return userLevelData;
		}
	}

	// The user does not exist yet, make a new one
	levelData[guildId].append(UserLevelData { userId });
	return levelData[guildId].back();
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

	QList<UserLevelData>& leaderboard = levelData[channel.guildId()];
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

	qSort(leaderboard.begin(), leaderboard.end(), [](const UserLevelData& first, const UserLevelData& second)
	{
		return first.currentXp > second.currentXp;
	});

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
	printf("Rank\n");
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
