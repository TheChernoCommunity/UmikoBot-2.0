#pragma once

#include "core/Core.h"
#include "modules/Module.h"

struct UserLevelData
{
	UserId userId = 0;
	int currentXp = 0;
	int messageCount = 0; // Resets every 30 seconds
};

class LevelModule : public Module
{
public:
	LevelModule();
	~LevelModule();

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&);

private:
	UserLevelData& getUserLevelData(GuildId guildId, UserId userId);
	
private:
	QTimer messageXpTimer;
	QMap<GuildId, QList<UserLevelData>> levelData;
};
