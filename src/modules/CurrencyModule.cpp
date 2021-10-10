#include "CurrencyModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <random>
#include <QtMath>

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
	registerCommand(Commands::Steal, "steal" UNSIGNED_DECIMAL USER, CommandPermission::User, CALLBACK(steal));
	registerCommand(Commands::Compensate, "compensate" DECIMAL OPTIONAL(USER), CommandPermission::Moderator, CALLBACK(compensate));
	registerCommand(Commands::Richlist, "richlist" OPTIONAL(UNSIGNED_INTEGER) OPTIONAL(UNSIGNED_INTEGER), CommandPermission::User, CALLBACK(richlist));
	registerCommand(Commands::Gamble, "gamble" OPTIONAL(UNSIGNED_DECIMAL), CommandPermission::User, CALLBACK(gamble));
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

void CurrencyModule::wallet(const Message& message, const Channel& channel)
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

void CurrencyModule::daily(const Message& message, const Channel& channel)
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

void CurrencyModule::donate(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	
	int amountInCents = args[1].toDouble() * 100;
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
	embed.setDescription(QString("Transferred **%1 %2** from the account of %3 to %4's balance!")
						 .arg(QString::number(amountInCents / 100.0f),
							  currencyConfigs[channel.guildId()].currencyAbbreviation,
							  UmikoBot::get().getName(channel.guildId(), senderId),
							  UmikoBot::get().getName(channel.guildId(), receiverId)));
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}

void CurrencyModule::steal(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	int amountInCents = args[1].toDouble() * 100;
	snowflake_t victimId = UmikoBot::get().getUserFromArgument(channel.guildId(), args[2]);
	snowflake_t thiefId = message.author().id();

	if (!victimId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}

	if (thiefId == victimId)
	{
		SEND_MESSAGE("You cannot steal from yourself!");
		return;
	}

	if (amountInCents == 0)
	{
		SEND_MESSAGE(QString("What are you even trying to steal? %1's dignity?").arg(UmikoBot::get().getName(channel.guildId(), victimId)));
		return;
	}

	// TODO(fkp): Check if victim is a bot

	// Just for ease of use
	UserCurrencyData& victimData = getUserCurrencyData(channel.guildId(), victimId);
	UserCurrencyData& thiefData = getUserCurrencyData(channel.guildId(), thiefId);
	const GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	
	if (victimData.balanceInCents - amountInCents < guildConfig.maxDebt)
	{
		SEND_MESSAGE("I can't let your victim go into serious debt!");
		return;
	}

	if (thiefData.balanceInCents - (amountInCents * guildConfig.stealFineAmount) < guildConfig.maxDebt)
	{
		SEND_MESSAGE("I can't let you go into serious debt!");
		return;
	}

	// https://www.desmos.com/calculator/lp80egcojn
	// This success chance is in the range of 0 to 1
	double successChance = guildConfig.stealSuccessBaseChance * qExp(-0.0001 * qPow(amountInCents / 100.0f, 1.5));
	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::uniform_real_distribution<> distribution { 0, 1 };

	if (distribution(prng) <= successChance)
	{
		// Steal success
		victimData.balanceInCents -= amountInCents;
		thiefData.balanceInCents += amountInCents;

		Embed embed;
		embed.setTitle(":man_detective: Steal Success! :man_detective:");
		embed.setColor(0x00ff00);
		embed.setDescription(QString("%1 has discreetly stolen **%2 %3** from right under %4's nose!")
							 .arg(UmikoBot::get().getName(channel.guildId(), thiefId),
								  QString::number(amountInCents / 100.0f), guildConfig.currencyAbbreviation,
								  UmikoBot::get().getName(channel.guildId(), victimId)));
		SEND_MESSAGE(embed);
	}
	else
	{
		// Steal failure
		victimData.balanceInCents += amountInCents * guildConfig.stealVictimBonus;
		thiefData.balanceInCents -= amountInCents * guildConfig.stealFineAmount;

		Embed embed;
		embed.setTitle(":rotating_light: You Got Caught! :rotating_light:");
		embed.setColor(0xff0000);
		embed.setDescription(QString("%1 has been fined **%2 %3** and placed in jail.\n" // TODO(fkp): Jail
									 "%4 has been granted **%5 %3** as compensation.")
							 .arg(UmikoBot::get().getName(channel.guildId(), thiefId),
								  QString::number((amountInCents * guildConfig.stealFineAmount) / 100.0f),
								  guildConfig.currencyAbbreviation,
								  UmikoBot::get().getName(channel.guildId(), victimId),
								  QString::number((amountInCents * guildConfig.stealVictimBonus) / 100.0f)));
		SEND_MESSAGE(embed);
	}
}

void CurrencyModule::compensate(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	int amountInCents = args[1].toDouble() * 100;
	const QString& currencyAbbreviation = currencyConfigs[channel.guildId()].currencyAbbreviation;

	if (args.size() == 2)
	{
		for (UserCurrencyData& userCurrencyData : currencyData[channel.guildId()])
		{
			userCurrencyData.balanceInCents += amountInCents;
		}

		SEND_MESSAGE(QString("Everyone has been compensated with **%1 %2**!").arg(QString::number(amountInCents / 100.0f), currencyAbbreviation));
	}
	else
	{
		snowflake_t userId = UmikoBot::get().getUserFromArgument(channel.guildId(), args[2]);
		if (!userId)
		{
			SEND_MESSAGE("Could not find user!");
			return;
		}

		getUserCurrencyData(channel.guildId(), userId).balanceInCents += amountInCents;
		SEND_MESSAGE(QString("%1 has been compensated with **%2 %3**!").arg(UmikoBot::get().getName(channel.guildId(), userId),
																			QString::number(amountInCents / 100.0f), currencyAbbreviation));
	}
}

void CurrencyModule::richlist(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	unsigned int min = 1;
	unsigned int max = 30;

	if (args.size() == 2)
	{
		max = args[1].toUInt();
	}
	else if (args.size() == 3)
	{	
		min = args[1].toUInt();
		max = args[2].toUInt();
	}

	if (min == 0 || max == 0)
	{
		SEND_MESSAGE("Your arguments must be greater than 0!");
		return;
	}

	QList<UserCurrencyData>& leaderboard = currencyData[channel.guildId()];
	if (min > leaderboard.size())
	{
		SEND_MESSAGE("Not enough members to create the list!");
		return;
	}
	if (max > leaderboard.size())
	{
		max = leaderboard.size();
	}
	if (min > max)
	{
		SEND_MESSAGE("The upper bound must be greater than the lower bound!");
		return;
	}

	qSort(leaderboard.begin(), leaderboard.end(), [](const UserCurrencyData& first, const UserCurrencyData& second)
	{
		return first.balanceInCents > second.balanceInCents;
	});

	QString description = "";
	unsigned int numberOfDigits = QString::number(max).size();
	unsigned int rank = min;

	for (unsigned int i = min; i <= max; i++)
	{
		QString name = UmikoBot::get().getName(channel.guildId(), leaderboard[i - 1].userId);
		if (name.isEmpty())
		{
			if (max < leaderboard.size())
			{
				max += 1;
			}

			continue;
		}

		rank += 1;
		description += QString("`%1`) **%2** - %3 %4\n").arg(QString::number(rank).rightJustified(numberOfDigits, ' '), name,
															 QString::number(leaderboard[i - 1].balanceInCents / 100.0f),
															 currencyConfigs[channel.guildId()].currencyAbbreviation);
	}

	Embed embed;
	embed.setTitle(QString("Currency Leaderboard (From %1 To %2)").arg(QString::number(min), QString::number(max)));
	embed.setDescription(description);
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}

void CurrencyModule::gamble(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	GuildGambleData& guildGambleData = gambleData[channel.guildId()];
	GuildCurrencyConfig& guildCurrencyConfig = currencyConfigs[channel.guildId()];

	if (guildGambleData.currentUser != 0)
	{
		SEND_MESSAGE(QString("Sorry, but this feature is currently being used by **%1**. Please try again later!")
					 .arg(UmikoBot::get().getName(channel.guildId(), guildGambleData.currentUser)));
		return;
	}

	if (args.size() == 1)
	{
		guildGambleData.amountBetInCents = guildCurrencyConfig.gambleDefaultAmountBet;
	}
	else
	{
		guildGambleData.amountBetInCents = args[1].toDouble() * 100;
		if (guildGambleData.amountBetInCents == 0)
		{
			SEND_MESSAGE("You must gamble *something*!");
			return;
		}
	}

	if (getUserCurrencyData(channel.guildId(), message.author().id()).balanceInCents - guildGambleData.amountBetInCents < guildCurrencyConfig.maxDebt)
	{
		SEND_MESSAGE("You're too poor to be gambling that much!");
		return;
	}

	// TODO(fkp): Idle timeout
	guildGambleData.currentUser = message.author().id();
	
	Embed embed;
	embed.setTitle(QString("%1 is Gambling").arg(UmikoBot::get().getName(channel.guildId(), guildGambleData.currentUser)));
	embed.setDescription(QString("All you need to do to win is to correctly guess a number between **1** and **5**!\n"
								 "Your Bet: **%1 %2**\n"
								 "Potential Winnings: **%3 %2**\n").arg(QString::number(guildGambleData.amountBetInCents / 100.0f),
																		guildCurrencyConfig.currencyAbbreviation,
																		QString::number(guildGambleData.amountBetInCents * 4 / 100.0f)));
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}
