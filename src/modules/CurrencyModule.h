#pragma once

#include "modules/Module.h"

struct UserCurrencyData
{
	snowflake_t userId;
	int balanceInCents = 0;

	bool hasClaimedDaily = false;
};

struct GuildCurrencyConfig
{
	QString currencyName = "PandaCoin";
	QString currencyAbbreviation = "PC";
	
	int rewardForDaily = 10000;
};

class CurrencyModule : public Module
{
public:
	CurrencyModule();
	~CurrencyModule();

	static void wallet(Module*, const Discord::Message&, const Discord::Channel&);
	static void daily(Module*, const Discord::Message&, const Discord::Channel&);

private:
	UserCurrencyData& getUserCurrencyData(snowflake_t guildId, snowflake_t userId);

private:
	QMap<snowflake_t /* guildId */, GuildCurrencyConfig> currencyConfigs;
	QMap<snowflake_t /* guildId */, QList<UserCurrencyData>> currencyData;

	QTimer dayTimer;
};
