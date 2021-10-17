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

	registerCommand(Commands::GiveXp, "give-xp" USER INTEGER OPTIONAL(SPACE "level" OPTIONAL("s")), CP::Moderator, CALLBACK(giveXp));
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

void LevelModule::giveXp(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	UserId userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[1]);
	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}
	
	int amountToAdd = args[2].toInt();
	UserLevelData& userLevelData = getUserLevelData(channel.guildId(), userId);
	long long int initialXp = userLevelData.currentXp;
	int initialLevel = getCurrentLevel(channel.guildId(), userId);

	if (args.size() == 3)
	{
		// Adds XP directly
		userLevelData.currentXp += amountToAdd;
	}
	else
	{
		// Adds a number of levels
		int currentLevel = getCurrentLevel(channel.guildId(), userId);

		for (int i = 1; i < amountToAdd + 1; i++)
		{
			if (currentLevel + i >= MAX_LEVEL)
			{
				break;
			}

			userLevelData.currentXp += levels[currentLevel + i];
		}
	}

	SEND_MESSAGE(QString("Added **%1 XP (%2 levels)** to %3!").arg(QString::number(userLevelData.currentXp - initialXp),
																   QString::number(getCurrentLevel(channel.guildId(), userId) - initialLevel),
																   UmikoBot::get().getName(channel.guildId(), userId)));
}
