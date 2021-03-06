#pragma once

#include "core/Core.h"
#include "modules/Module.h"

struct UserCurrencyData
{
	UserId userId;
	int balanceInCents = 0;

	bool hasClaimedDaily = false;
	int dailyStreak = 0;
	
	QTimer* jailTimer = nullptr;
	bool hasUsedBribe = false;

	// Achievements
	int numberOfDailysClaimed = 0;
	int longestDailyStreak = 0;
	int numberOfGiveawaysClaimed = 0;
	int amountDonatedInCents = 0;
	int amountReceivedFromDonationsInCents = 0;
	int amountStolenInCents = 0;
	int netAmountFromGamblingInCents = 0;
	int amountSpentOnBribingInCents = 0;
};

struct GuildCurrencyConfig
{
	QString currencyName = "PandaCoin";
	QString currencyAbbreviation = "PC";
	
	int maxDebt = -10000;
	
	int rewardForDaily = 10000;
	int dailyStreakBonus = 5000;
	int dailyStreakBonusPeriod = 3;

	double randomGiveawayChance = 0.0005; // assuming approximately 2000 messages per day
	int randomGiveawayReward = 30000;
	bool randomGiveawayDone = false;
	bool randomGiveawayInProgress = false;
	UserId randomGiveawayClaimer = 0;
	QTimer* randomGiveawayTimer = nullptr;
	
	double stealSuccessBaseChance = 0.4;
	double stealFineAmount = 0.5; // Portion of attempted steal amount
	double stealVictimBonus = 0.25; // Portion of attempted steal amount
	int stealJailTimeMinutes = 60;

	double bribeMinSuccessChance = 0.3;
	double bribeMaxSuccessChance = 0.7;
	int bribeMinAmountInCents = 2000;
	int bribeMaxAmountInCents = 15000;
	int bribeExtraJailTimeMinutes = 60;
	
	int gambleDefaultAmountBet = 2500;
	int gambleTimeoutSeconds = 20;
};

struct GuildGambleData
{
	UserId currentUserId = 0;
	int amountBetInCents;
	QTimer* idleTimeoutTimer = nullptr;
};

class CurrencyModule : public Module
{
public:
	CurrencyModule();

	void wallet(const Discord::Message&, const Discord::Channel&);
	void daily(const Discord::Message&, const Discord::Channel&);
	void donate(const Discord::Message&, const Discord::Channel&);
	void steal(const Discord::Message&, const Discord::Channel&);
	void compensate(const Discord::Message&, const Discord::Channel&);
	void richlist(const Discord::Message&, const Discord::Channel&);
	void gamble(const Discord::Message&, const Discord::Channel&);
	void bribe(const Discord::Message&, const Discord::Channel&);
	void claim(const Discord::Message&, const Discord::Channel&);

	void setCurrencyName(const Discord::Message&, const Discord::Channel&);
	void setMaxDebt(const Discord::Message&, const Discord::Channel&);
	void setDailyReward(const Discord::Message&, const Discord::Channel&);
	void setDailyStreakBonus(const Discord::Message&, const Discord::Channel&);
	void setDailyStreakBonusPeriod(const Discord::Message&, const Discord::Channel&);
	void setRandomGiveawayChance(const Discord::Message&, const Discord::Channel&);
	void setRandomGiveawayReward(const Discord::Message&, const Discord::Channel&);
	void setStealSuccessChance(const Discord::Message&, const Discord::Channel&);
	void setStealFine(const Discord::Message&, const Discord::Channel&);
	void setStealVictimBonus(const Discord::Message&, const Discord::Channel&);
	void setStealJailTime(const Discord::Message&, const Discord::Channel&);
	void setBribeSuccessChance(const Discord::Message&, const Discord::Channel&);
	void setBribeAmount(const Discord::Message&, const Discord::Channel&);
	void setBribeExtraJailTime(const Discord::Message&, const Discord::Channel&);
	void setGambleDefaultBet(const Discord::Message&, const Discord::Channel&);
	void setGambleTimeout(const Discord::Message&, const Discord::Channel&);
	
	UserCurrencyData& getUserCurrencyData(GuildId guildId, UserId userId);
	GuildCurrencyConfig& getGuildCurrencyConfig(GuildId guildId);

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&) override;
	void onStatus(QString& output, GuildId guildId, UserId userId) override;
	
private:
	QMap<GuildId, GuildCurrencyConfig> currencyConfigs;
	QMap<GuildId, QMap<UserId, UserCurrencyData>> currencyData;
	QMap<GuildId, GuildGambleData> gambleData;

	QTimer dayTimer;
};
