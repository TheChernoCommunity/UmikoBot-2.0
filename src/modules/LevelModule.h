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

struct LevelRank
{
	QString name = "";
	unsigned int minimumLevel = 0;
};

class LevelModule : public Module
{
public:
	LevelModule();

	void top(const Discord::Message&, const Discord::Channel&);
	void giveXp(const Discord::Message&, const Discord::Channel&);
	void takeXp(const Discord::Message&, const Discord::Channel&);
	void rank(const Discord::Message&, const Discord::Channel&);
	void enableXp(const Discord::Message&, const Discord::Channel&);
	void disableXp(const Discord::Message&, const Discord::Channel&);
	
protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&) override;
	void onStatus(QString& output, GuildId guildId, UserId userId) override;

private:
	UserLevelData& getUserLevelData(GuildId guildId, UserId userId);
	void generateLevels();
	int getCurrentLevel(GuildId guildId, UserId userId);
	QString getCurrentRank(GuildId guildId, UserId userId);

	void giveTakeXpImpl(const Discord::Message&, const Discord::Channel&, int multiplier);
	void enableDisableXpImpl(const Discord::Message&, const Discord::Channel&, bool enable);
	
	void sortRanks(GuildId guildId);
	QList<UserLevelData> getLeaderboard(GuildId guildId);

private:
	// We need to have enabled being the default, so:
	// true = disabled, false = enabled
	QMap<ChannelId, bool> channelsWithXpDisabled;
	
	QList<long long int> levels; // index is level number, value is individual XP requirement (not cumulative)
	QMap<GuildId, QList<LevelRank>> levelRanks;
	QMap<GuildId, QMap<UserId, UserLevelData>> levelData;
	
	QTimer messageXpTimer;
};
