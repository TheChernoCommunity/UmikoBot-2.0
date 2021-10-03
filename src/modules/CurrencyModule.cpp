#include "CurrencyModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

CurrencyModule::CurrencyModule()
	: Module("Currency")
{
	registerCommand(Commands::Daily, "daily", CommandPermission::User, daily);
}

CurrencyModule::~CurrencyModule()
{
}

UserCurrencyData& CurrencyModule::getUserCurrencyData(snowflake_t guildId, snowflake_t userId)
{
	for (UserCurrencyData& userCurrencyData : currencyData[guildId])
	{
		if (userCurrencyData.userId == userId)
		{
			return userCurrencyData;
		}
	}

	// The user does not exist yet, make a new one
	currencyData[guildId].append(UserCurrencyData { userId });
	return currencyData[guildId].back();
}

void CurrencyModule::daily(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	CurrencyModule* self = (CurrencyModule*) module;
	UserCurrencyData& userCurrencyData = self->getUserCurrencyData(channel.guildId(), message.author().id());
	const GuildCurrencyConfig& guildCurrencyConfig = self->currencyConfigs[channel.guildId()];

	if (userCurrencyData.hasClaimedDaily)
	{
		// TODO(fkp): Time left
		SEND_MESSAGE("You have already claimed your credits for today!");
		return;
	}

	userCurrencyData.balance += guildCurrencyConfig.rewardForDaily;
	userCurrencyData.hasClaimedDaily = true;
	
	QString output = QString("You now have **%1** more %2s in your wallet!").arg(QString::number(guildCurrencyConfig.rewardForDaily / 100.0f),
																				 guildCurrencyConfig.currencyName);
	SEND_MESSAGE(output);
}
