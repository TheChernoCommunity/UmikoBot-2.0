#pragma once

#include "modules/Module.h"

struct UserCurrencyData
{
	snowflake_t userId;
	int balanceInCents = 0;

	bool hasClaimedDaily = false;
	QTimer* jailTimer = nullptr;
};

struct GuildCurrencyConfig
{
	QString currencyName = "PandaCoin";
	QString currencyAbbreviation = "PC";
	
	int rewardForDaily = 10000;
	int maxDebt = -10000;

	double stealSuccessBaseChance = 0.4;
	double stealFineAmount = 0.5; // Portion of attempted steal amount
	double stealVictimBonus = 0.25; // Portion of attempted steal amount
	int stealJailTimeMinutes = 60;

	int gambleDefaultAmountBet = 2500;
	int gambleTimeoutSeconds = 20;
};

struct GuildGambleData
{
	snowflake_t currentUserId = 0;
	int amountBetInCents;
	QTimer* idleTimeoutTimer = nullptr;
};

class CurrencyModule : public Module
{
public:
	CurrencyModule();
	~CurrencyModule();

	void wallet(const Discord::Message&, const Discord::Channel&);
	void daily(const Discord::Message&, const Discord::Channel&);
	void donate(const Discord::Message&, const Discord::Channel&);
	void steal(const Discord::Message&, const Discord::Channel&);
	void compensate(const Discord::Message&, const Discord::Channel&);
	void richlist(const Discord::Message&, const Discord::Channel&);
	void gamble(const Discord::Message&, const Discord::Channel&);

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&) override;
	
private:
	UserCurrencyData& getUserCurrencyData(snowflake_t guildId, snowflake_t userId);

private:
	QMap<snowflake_t /* guildId */, GuildCurrencyConfig> currencyConfigs;
	QMap<snowflake_t /* guildId */, QList<UserCurrencyData>> currencyData;
	QMap<snowflake_t /* guildId */, GuildGambleData> gambleData;

	QTimer dayTimer;
};
