#pragma once

#include "core/Core.h"
#include "modules/Module.h"

// TODO(fkp): Make not macros
#define MAX_LEVEL 150
#define LEVEL_0_XP_REQUIREMENT 100
#define XP_REQUIREMENT_GROWTH_RATE 1.12

struct UserLevelData
{
	UserId userId = 0;
	long long int currentXp = 0;
	int messageCount = 0; // Resets every 30 seconds
};

class LevelModule : public Module
{
public:
	LevelModule();
	~LevelModule();

	void giveXp(const Discord::Message&, const Discord::Channel&);
	
protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&);

private:
	UserLevelData& getUserLevelData(GuildId guildId, UserId userId);
	void generateLevels();
	int getCurrentLevel(GuildId guildId, UserId userId);
	
private:
	QTimer messageXpTimer;
	QList<long long int> levels; // index is level number, value is individual XP requirement (not cumulative)
	QMap<GuildId, QList<UserLevelData>> levelData;
};
