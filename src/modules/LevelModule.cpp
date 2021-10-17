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
	
	namespace CP = CommandPermission;
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
