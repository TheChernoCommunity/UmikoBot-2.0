#include "CurrencyModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

CurrencyModule::CurrencyModule()
	: Module("Currency")
{
	dayTimer.setInterval(24 * 60 * 60 * 1000);
	dayTimer.start();
	QObject::connect(&dayTimer, &QTimer::timeout, [this]()
	{
		// Resets daily collection timeout and removes users no longer in the server
		for (snowflake_t guildId : currencyData.keys())
		{
			for (int userIndex = 0; userIndex < currencyData[guildId].size(); userIndex++)
			{
				UserCurrencyData& userCurrencyData = currencyData[guildId][userIndex];
				userCurrencyData.hasClaimedDaily = false;
				// TODO(fkp): Each module should be able to react to guild member remove
			}
		}
	});
	
	registerCommand(Commands::Wallet, "wallet" OPTIONAL(USER), CommandPermission::User, CALLBACK(wallet));
	registerCommand(Commands::Daily, "daily", CommandPermission::User, CALLBACK(daily));
	registerCommand(Commands::Donate, "donate" UNSIGNED_DECIMAL USER, CommandPermission::User, CALLBACK(donate));
}

CurrencyModule::~CurrencyModule()
{
}

void CurrencyModule::onSave(QJsonObject& mainObject) const
{
	for (snowflake_t guildId : currencyData.keys())
	{
		QJsonObject guildJson {};
		for (const UserCurrencyData& userCurrencyData : currencyData[guildId])
		{
			QJsonObject userJson {};
			userJson["balanceInCents"] = userCurrencyData.balanceInCents;
			userJson["hasClaimedDaily"] = userCurrencyData.hasClaimedDaily;

			guildJson[QString::number(userCurrencyData.userId)] = userJson;
		}

		mainObject[QString::number(guildId)] = guildJson;
	}
}

void CurrencyModule::onLoad(const QJsonObject& mainObject)
{
	for (const QString& guildIdString : mainObject.keys())
	{
		QJsonObject guildJson = mainObject[guildIdString].toObject();
		snowflake_t guildId = guildIdString.toULongLong();

		for (const QString& userIdString : guildJson.keys())
		{
			QJsonObject userJson = guildJson[userIdString].toObject();
			snowflake_t userId = userIdString.toULongLong();
			
			currencyData[guildId].append(UserCurrencyData {
				userId,
				userJson["balanceInCents"].toInt(),
				userJson["hasClaimedDaily"].toBool(),
			});
		}
	}
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

void CurrencyModule::wallet(const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	snowflake_t userId = 0;

	if (args.size() == 1)
	{
		userId = message.author().id();
	}
	else
	{
		userId = UmikoBot::get().getUserFromArgument(channel.guildId(), args[1]);
	}

	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}

	UmikoBot::get().getAvatar(channel.guildId(), userId).then([this, message, channel, userId](const QString& icon)
	{
		const GuildCurrencyConfig& guildCurrencyConfig = currencyConfigs[channel.guildId()];
		QString desc = QString("Current %1s: **%2 %3**").arg(guildCurrencyConfig.currencyName,
															QString::number(getUserCurrencyData(channel.guildId(), userId).balanceInCents / 100.0f),
															guildCurrencyConfig.currencyAbbreviation);

		Embed embed;
		embed.setColor(qrand() % 0xffffff);
		embed.setAuthor(EmbedAuthor { UmikoBot::get().getName(channel.guildId(), userId) + "'s Wallet", "", icon });
		embed.setDescription(desc);
		SEND_MESSAGE(embed);
	});
}

void CurrencyModule::daily(const Discord::Message& message, const Discord::Channel& channel)
{
	UserCurrencyData& userCurrencyData = getUserCurrencyData(channel.guildId(), message.author().id());
	const GuildCurrencyConfig& guildCurrencyConfig = currencyConfigs[channel.guildId()];

	if (userCurrencyData.hasClaimedDaily)
	{
		// TODO(fkp): Time left
		SEND_MESSAGE("You have already claimed your credits for today!");
		return;
	}

	userCurrencyData.balanceInCents += guildCurrencyConfig.rewardForDaily;
	userCurrencyData.hasClaimedDaily = true;
	
	QString output = QString("You now have **%1** more %2s in your wallet!").arg(QString::number(guildCurrencyConfig.rewardForDaily / 100.0f),
																				 guildCurrencyConfig.currencyName);
	SEND_MESSAGE(output);
}

void CurrencyModule::donate(const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	
	int amountInCents = (args[1].toDouble() * 100);
	snowflake_t receiverId = UmikoBot::get().getUserFromArgument(channel.guildId(), args[2]);
	snowflake_t senderId = message.author().id();

	if (!receiverId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}

	if (senderId == receiverId)
	{
		SEND_MESSAGE("You cannot donate to yourself!");
		return;
	}

	if (amountInCents == 0)
	{
		SEND_MESSAGE("Why waste my time with feeble donations?");
		return;
	}

	if (getUserCurrencyData(channel.guildId(), senderId).balanceInCents - amountInCents < currencyConfigs[channel.guildId()].maxDebt)
	{
		SEND_MESSAGE("You're too poor to be donating!");
		return;
	}
	
	getUserCurrencyData(channel.guildId(), senderId).balanceInCents -= amountInCents;
	getUserCurrencyData(channel.guildId(), receiverId).balanceInCents += amountInCents;

	Embed embed {};
	embed.setTitle("Donation by " + UmikoBot::get().getName(channel.guildId(), senderId));
	embed.setDescription(QString("Transferred %1%2 from the account of %3 to %4's balance!")
						 .arg(QString::number(amountInCents / 100.0f),
							  currencyConfigs[channel.guildId()].currencyAbbreviation,
							  UmikoBot::get().getName(channel.guildId(), senderId),
							  UmikoBot::get().getName(channel.guildId(), receiverId)));
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}
