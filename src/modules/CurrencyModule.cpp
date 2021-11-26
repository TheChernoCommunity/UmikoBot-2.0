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
		for (GuildId guildId : currencyData.keys())
		{
			GuildCurrencyConfig& guildConfig = currencyConfigs[guildId];
			
			for (UserCurrencyData& userCurrencyData : currencyData[guildId])
			{
				if (!userCurrencyData.hasClaimedDaily)
				{
					userCurrencyData.dailyStreak = 0;
				}
				
				userCurrencyData.hasClaimedDaily = false;
			}
			
			if (!guildConfig.randomGiveawayDone)
			{
				guildConfig.randomGiveawayInProgress = true;
				UmikoBot::get().createMessage(UmikoBot::get().primaryChannels[guildId],
											  QString("Hey everyone! Today's freebie expires in **60 seconds**! Go `%1claim` it now!")
											  .arg(UmikoBot::get().getGuildData()[guildId].prefix));
			}

			if (!guildConfig.randomGiveawayTimer)
			{
				guildConfig.randomGiveawayTimer = new QTimer();
			}
			guildConfig.randomGiveawayTimer->setSingleShot(true);
			guildConfig.randomGiveawayTimer->setInterval(60 * 1000);
			guildConfig.randomGiveawayTimer->start();
			QObject::connect(guildConfig.randomGiveawayTimer, &QTimer::timeout, [this, guildId]()
			{
				currencyConfigs[guildId].randomGiveawayInProgress = false;
				currencyConfigs[guildId].randomGiveawayDone = false;
				currencyConfigs[guildId].randomGiveawayClaimer = 0;
			});
		}
	});

	namespace CP = CommandPermission;
	
	registerCommand(Commands::Wallet, "wallet" OPTIONAL(USER), CP::User, CALLBACK(wallet));
	registerCommand(Commands::Daily, "daily", CP::User, CALLBACK(daily));
	registerCommand(Commands::Donate, "donate" UNSIGNED_DECIMAL USER, CP::User, CALLBACK(donate));
	registerCommand(Commands::Steal, "steal" UNSIGNED_DECIMAL USER, CP::User, CALLBACK(steal));
	registerCommand(Commands::Compensate, "compensate" DECIMAL OPTIONAL(USER), CP::Moderator, CALLBACK(compensate));
	registerCommand(Commands::Richlist, "richlist" OPTIONAL(UNSIGNED_INTEGER) OPTIONAL(UNSIGNED_INTEGER), CP::User, CALLBACK(richlist));
	registerCommand(Commands::Gamble, "gamble" OPTIONAL(UNSIGNED_DECIMAL), CP::User, CALLBACK(gamble));
	registerCommand(Commands::Bribe, "bribe" UNSIGNED_DECIMAL, CP::User, CALLBACK(bribe));
	registerCommand(Commands::Claim, "claim", CP::User, CALLBACK(claim));

	registerCommand(Commands::SetCurrencyName, "set-currency-name" OPTIONAL(IDENTIFIER IDENTIFIER), CP::Moderator, CALLBACK(setCurrencyName));
	registerCommand(Commands::SetMaxDebt, "set-max-debt" OPTIONAL(DECIMAL), CP::Moderator, CALLBACK(setMaxDebt));
	registerCommand(Commands::SetDailyReward, "set-daily-reward" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setDailyReward));
	registerCommand(Commands::SetDailyStreakBonus, "set-daily-streak-bonus" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setDailyStreakBonus));
	registerCommand(Commands::SetDailyStreakBonusPeriod, "set-daily-streak-bonus-period" OPTIONAL(UNSIGNED_INTEGER), CP::Moderator, CALLBACK(setDailyStreakBonusPeriod));
	registerCommand(Commands::SetRandomGiveawayChance, "set-random-giveaway-chance" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setRandomGiveawayChance));
	registerCommand(Commands::SetRandomGiveawayReward, "set-random-giveaway-reward" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setRandomGiveawayReward));
	registerCommand(Commands::SetStealSuccessChance, "set-steal-success-chance" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setStealSuccessChance));
	registerCommand(Commands::SetStealFine, "set-steal-fine" OPTIONAL(DECIMAL), CP::Moderator, CALLBACK(setStealFine));
	registerCommand(Commands::SetStealVictimBonus, "set-steal-victim-bonus" OPTIONAL(DECIMAL), CP::Moderator, CALLBACK(setStealVictimBonus));
	registerCommand(Commands::SetStealJailTime, "set-steal-jail-time" OPTIONAL(UNSIGNED_INTEGER), CP::Moderator, CALLBACK(setStealJailTime));
	registerCommand(Commands::SetBribeSuccessChance, "set-bribe-success-chance" OPTIONAL(UNSIGNED_DECIMAL UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setBribeSuccessChance));
	registerCommand(Commands::SetBribeAmount, "set-bribe-amount" OPTIONAL(DECIMAL DECIMAL), CP::Moderator, CALLBACK(setBribeAmount));
	registerCommand(Commands::SetBribeExtraJailTime, "set-bribe-extra-jail-time" OPTIONAL(UNSIGNED_INTEGER), CP::Moderator, CALLBACK(setBribeExtraJailTime));
	registerCommand(Commands::SetGambleDefaultBet, "set-gamble-default-bet" OPTIONAL(UNSIGNED_DECIMAL), CP::Moderator, CALLBACK(setGambleDefaultBet));
	registerCommand(Commands::SetGambleTimeout, "set-gamble-timeout" OPTIONAL(UNSIGNED_INTEGER), CP::Moderator, CALLBACK(setGambleTimeout));
}

void CurrencyModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject guildConfigObject {};
	QJsonObject userDataObject {};
	
	for (GuildId guildId : currencyConfigs.keys())
	{
		QJsonObject guildJson {};
		guildJson["currencyName"] = currencyConfigs[guildId].currencyName;
		guildJson["currencyAbbreviation"] = currencyConfigs[guildId].currencyAbbreviation;
		
		guildJson["maxDebt"] = currencyConfigs[guildId].maxDebt;
		
		guildJson["rewardForDaily"] = currencyConfigs[guildId].rewardForDaily;
		guildJson["dailyStreakBonus"] = currencyConfigs[guildId].dailyStreakBonus;
		guildJson["dailyStreakBonusPeriod"] = currencyConfigs[guildId].dailyStreakBonusPeriod;

		guildJson["randomGiveawayChance"] = currencyConfigs[guildId].randomGiveawayChance;
		guildJson["randomGiveawayReward"] = currencyConfigs[guildId].randomGiveawayReward;

		guildJson["stealSuccessBaseChance"] = currencyConfigs[guildId].stealSuccessBaseChance;
		guildJson["stealFineAmount"] = currencyConfigs[guildId].stealFineAmount;
		guildJson["stealVictimBonus"] = currencyConfigs[guildId].stealVictimBonus;
		guildJson["stealJailTimeMinutes"] = currencyConfigs[guildId].stealJailTimeMinutes;
		
		guildJson["bribeMinSuccessChance"] = currencyConfigs[guildId].bribeMinSuccessChance;
		guildJson["bribeMaxSuccessChance"] = currencyConfigs[guildId].bribeMaxSuccessChance;
		guildJson["bribeMinAmountInCents"] = currencyConfigs[guildId].bribeMinAmountInCents;
		guildJson["bribeMaxAmountInCents"] = currencyConfigs[guildId].bribeMaxAmountInCents;
		guildJson["bribeExtraJailTimeMinutes"] = currencyConfigs[guildId].bribeExtraJailTimeMinutes;
		
		guildJson["gambleDefaultAmountBet"] = currencyConfigs[guildId].gambleDefaultAmountBet;
		guildJson["gambleTimeoutSeconds"] = currencyConfigs[guildId].gambleTimeoutSeconds;

		guildConfigObject[QString::number(guildId)] = guildJson;
	}

	for (GuildId guildId : currencyData.keys())
	{
		QJsonObject guildJson {};
		for (const UserCurrencyData& userCurrencyData : currencyData[guildId])
		{
			QJsonObject userJson {};
			userJson["balanceInCents"] = userCurrencyData.balanceInCents;
			userJson["hasClaimedDaily"] = userCurrencyData.hasClaimedDaily;
			userJson["dailyStreak"] = userCurrencyData.dailyStreak;

			userJson["numberOfDailysClaimed"] = userCurrencyData.numberOfDailysClaimed;
			userJson["longestDailyStreak"] = userCurrencyData.longestDailyStreak;
			userJson["numberOfGiveawaysClaimed"] = userCurrencyData.numberOfGiveawaysClaimed;
			userJson["amountDonatedInCents"] = userCurrencyData.amountDonatedInCents;
			userJson["amountReceivedFromDonationsInCents"] = userCurrencyData.amountReceivedFromDonationsInCents;
			userJson["amountStolenInCents"] = userCurrencyData.amountStolenInCents;
			userJson["netAmountFromGamblingInCents"] = userCurrencyData.netAmountFromGamblingInCents;
			userJson["amountSpentOnBribingInCents"] = userCurrencyData.amountSpentOnBribingInCents;

			guildJson[QString::number(userCurrencyData.userId)] = userJson;
		}

		userDataObject[QString::number(guildId)] = guildJson;
	}

	mainObject["guildConfig"] = guildConfigObject;
	mainObject["userData"] = userDataObject;
}

void CurrencyModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject guildConfigObject = mainObject["guildConfig"].toObject();
	QJsonObject userDataObject = mainObject["userData"].toObject();

	for (const QString& guildIdString : guildConfigObject.keys())
	{
		QJsonObject guildJson = guildConfigObject[guildIdString].toObject();
		GuildId guildId = guildIdString.toULongLong();

		currencyConfigs[guildId].currencyName = guildJson["currencyName"].toString();
		currencyConfigs[guildId].currencyAbbreviation = guildJson["currencyAbbreviation"].toString();
		
		currencyConfigs[guildId].maxDebt = guildJson["maxDebt"].toInt();

		currencyConfigs[guildId].rewardForDaily = guildJson["rewardForDaily"].toInt();
		currencyConfigs[guildId].dailyStreakBonus = guildJson["dailyStreakBonus"].toInt();
		currencyConfigs[guildId].dailyStreakBonusPeriod = guildJson["dailyStreakBonusPeriod"].toInt();
		
		currencyConfigs[guildId].randomGiveawayChance = guildJson["randomGiveawayChance"].toDouble();
		currencyConfigs[guildId].randomGiveawayReward = guildJson["randomGiveawayReward"].toInt();

		currencyConfigs[guildId].stealSuccessBaseChance = guildJson["stealSuccessBaseChance"].toDouble();
		currencyConfigs[guildId].stealFineAmount = guildJson["stealFineAmount"].toDouble();
		currencyConfigs[guildId].stealVictimBonus = guildJson["stealVictimBonus"].toDouble();
		currencyConfigs[guildId].stealJailTimeMinutes = guildJson["stealJailTimeMinutes"].toInt();
		
		currencyConfigs[guildId].bribeMinSuccessChance = guildJson["bribeMinSuccessChance"].toDouble();
		currencyConfigs[guildId].bribeMaxSuccessChance = guildJson["bribeMaxSuccessChance"].toDouble();
		currencyConfigs[guildId].bribeMinAmountInCents = guildJson["bribeMinAmountInCents"].toInt();
		currencyConfigs[guildId].bribeMaxAmountInCents = guildJson["bribeMaxAmountInCents"].toInt();
		currencyConfigs[guildId].bribeExtraJailTimeMinutes = guildJson["bribeExtraJailTimeMinutes"].toInt();
		
		currencyConfigs[guildId].gambleDefaultAmountBet = guildJson["gambleDefaultAmountBet"].toInt();
		currencyConfigs[guildId].gambleTimeoutSeconds = guildJson["gambleTimeoutSeconds"].toInt();
	}
	
	for (const QString& guildIdString : userDataObject.keys())
	{
		QJsonObject guildJson = userDataObject[guildIdString].toObject();
		GuildId guildId = guildIdString.toULongLong();

		for (const QString& userIdString : guildJson.keys())
		{
			QJsonObject userJson = guildJson[userIdString].toObject();
			UserId userId = userIdString.toULongLong();
			
			currencyData[guildId][userId] = UserCurrencyData {
				userId,
				userJson["balanceInCents"].toInt(),
				userJson["hasClaimedDaily"].toBool(),
				userJson["dailyStreak"].toInt(),
				nullptr,
				false,

				userJson["numberOfDailysClaimed"].toInt(),
				userJson["longestDailyStreak"].toInt(),
				userJson["numberOfGiveawaysClaimed"].toInt(),
				userJson["amountDonatedInCents"].toInt(),
				userJson["amountReceivedFromDonationsInCents"].toInt(),
				userJson["amountStolenInCents"].toInt(),
				userJson["netAmountFromGamblingInCents"].toInt(),
				userJson["amountSpentOnBribingInCents"].toInt(),
			};
		}
	}
}

void CurrencyModule::onMessage(const Message& message, const Channel& channel)
{
	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };

	// Random giveaway
	GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	if (!guildConfig.randomGiveawayDone && !guildConfig.randomGiveawayInProgress)
	{
		std::uniform_real_distribution<> distribution { 0, 1 };
		if (distribution(prng) <= guildConfig.randomGiveawayChance)
		{
			guildConfig.randomGiveawayInProgress = true;
			UmikoBot::get().createMessage(UmikoBot::get().primaryChannels[channel.guildId()],
										  QString("Hey everyone! **FREEBIE** available now! Go `%1claim` some juicy coins!")
										  .arg(UmikoBot::get().getGuildData()[channel.guildId()].prefix));
		}
	}
	
	// Gamble game
	if (gambleData[channel.guildId()].currentUserId == message.author().id())
	{
		unsigned int guess = message.content().toUInt(); // Returns 0 if failed

		if (guess == 0 || guess > 5)
		{
			SEND_MESSAGE("Your guess must be a number between **1** and **5**!");
			return;
		}

		std::uniform_int_distribution<> distribution { 1, 5 };
		
		Embed embed;

		if ((unsigned int) distribution(prng) == guess)
		{
			int amountWon = gambleData[channel.guildId()].amountBetInCents * 4;
			getUserCurrencyData(channel.guildId(), gambleData[channel.guildId()].currentUserId).balanceInCents += amountWon;
			getUserCurrencyData(channel.guildId(), gambleData[channel.guildId()].currentUserId).netAmountFromGamblingInCents += amountWon;
			
			embed.setColor(0x00ff00);
			embed.setTitle("You Won!!!");
			embed.setDescription(QString("Congrats! **%1 %2** has been placed in your bank account!")
								 .arg(QString::number(amountWon / 100.0f), guildConfig.currencyAbbreviation));
		}
		else
		{
			int amountLost = gambleData[channel.guildId()].amountBetInCents;
			getUserCurrencyData(channel.guildId(), gambleData[channel.guildId()].currentUserId).balanceInCents -= amountLost;
			getUserCurrencyData(channel.guildId(), gambleData[channel.guildId()].currentUserId).netAmountFromGamblingInCents -= amountLost;
			
			embed.setColor(0xff0000);
			embed.setTitle("You Lost!");
			embed.setDescription(QString("Better luck next time! **%1 %2** has been taken from your account.")
								 .arg(QString::number(amountLost / 100.0f), guildConfig.currencyAbbreviation));
		}

		gambleData[channel.guildId()].currentUserId = 0;
		delete gambleData[channel.guildId()].idleTimeoutTimer;
		gambleData[channel.guildId()].idleTimeoutTimer = nullptr;
		
		SEND_MESSAGE(embed);
	}
}

void CurrencyModule::onStatus(QString& output, GuildId guildId, UserId userId)
{
	output += QString("Wallet: %1 %2\n").arg(QString::number(getUserCurrencyData(guildId, userId).balanceInCents / 100.0f),
											 currencyConfigs[guildId].currencyAbbreviation);
	output += "\n";
}

UserCurrencyData& CurrencyModule::getUserCurrencyData(GuildId guildId, UserId userId)
{
	currencyData[guildId][userId].userId = userId; // Just in case this is first initialisation
	return currencyData[guildId][userId];
}

GuildCurrencyConfig& CurrencyModule::getGuildCurrencyConfig(GuildId guildId)
{
	return currencyConfigs[guildId];
}

void CurrencyModule::wallet(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	UserId userId = 0;

	if (args.size() == 1)
	{
		userId = message.author().id();
	}
	else
	{
		userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[1]);
	}

	if (!userId)
	{
		SEND_MESSAGE("Could not find user!");
		return;
	}

	UmikoBot::get().getAvatar(channel.guildId(), userId).then([this, message, channel, userId](const QString& icon)
	{
		const UserCurrencyData& userData = getUserCurrencyData(channel.guildId(), userId);
		const GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
		int streakDaysLeft = userData.dailyStreak % guildConfig.dailyStreakBonusPeriod;
		
		QString desc = QString("Current %1s: **%2 %3**\n").arg(guildConfig.currencyName, QString::number(userData.balanceInCents / 100.0f),
															 guildConfig.currencyAbbreviation);
		desc += QString("Daily Streak: **%1/%2**\n").arg(QString::number(userData.dailyStreak),
													   QString::number(userData.dailyStreak + (guildConfig.dailyStreakBonusPeriod - streakDaysLeft)));
		desc += QString("Today's Daily Claimed? **%1**\n").arg(userData.hasClaimedDaily ? "Yes" : "No");

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
	const GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];

	if (userCurrencyData.hasClaimedDaily)
	{
		SEND_MESSAGE(QString("You have already claimed your credits for today!\nCome back in **%1** to collect more!")
					 .arg(stringifyMilliseconds(dayTimer.remainingTime())));
		return;
	}

	if (userCurrencyData.jailTimer)
	{
		SEND_MESSAGE(QString("You are in jail!\nYou will be released in **%1**!").arg(stringifyMilliseconds(userCurrencyData.jailTimer->remainingTime())));
		return;
	}

	userCurrencyData.balanceInCents += guildConfig.rewardForDaily;
	userCurrencyData.hasClaimedDaily = true;
	userCurrencyData.dailyStreak += 1;
	userCurrencyData.longestDailyStreak = qMax(userCurrencyData.longestDailyStreak, userCurrencyData.dailyStreak);
	userCurrencyData.numberOfDailysClaimed += 1;
	
	QString output;
	int streakDaysLeft = userCurrencyData.dailyStreak % guildConfig.dailyStreakBonusPeriod;

	if (streakDaysLeft == 0)
	{
		userCurrencyData.balanceInCents += guildConfig.dailyStreakBonus;
		output = QString("**Bonus!!!** You now have **%1** more %2s in your wallet!\n"
						 "Streak: **%3**").arg(QString::number((guildConfig.rewardForDaily + guildConfig.dailyStreakBonus) / 100.0f),
											   guildConfig.currencyName, QString::number(userCurrencyData.dailyStreak));
	}
	else
	{
		output = QString("You now have **%1** more %2s in your wallet!\n"
						 "Streak: **%3/%4**").arg(QString::number(guildConfig.rewardForDaily / 100.0f), guildConfig.currencyName,
												  QString::number(userCurrencyData.dailyStreak),
												  QString::number(userCurrencyData.dailyStreak + (guildConfig.dailyStreakBonusPeriod - streakDaysLeft)));
	}

	SEND_MESSAGE(output);
}

void CurrencyModule::donate(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	
	int amountInCents = args[1].toDouble() * 100;
	UserId receiverId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[2]);
	UserId senderId = message.author().id();

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

	UmikoBot::get().getUser(receiverId).then([this, message, channel, senderId, receiverId, amountInCents](const User& user)
	{
		if (user.bot())
		{
			SEND_MESSAGE("Bots have no need for your worthless currency!");
			return;
		}

		getUserCurrencyData(channel.guildId(), senderId).balanceInCents -= amountInCents;
		getUserCurrencyData(channel.guildId(), senderId).amountDonatedInCents += amountInCents;
		getUserCurrencyData(channel.guildId(), receiverId).balanceInCents += amountInCents;
		getUserCurrencyData(channel.guildId(), receiverId).amountReceivedFromDonationsInCents += amountInCents;

		Embed embed {};
		embed.setTitle("Donation by " + UmikoBot::get().getName(channel.guildId(), senderId));
		embed.setDescription(QString("Transferred **%1 %2** from the account of %3 to %4's balance!")
							 .arg(QString::number(amountInCents / 100.0f),
								  currencyConfigs[channel.guildId()].currencyAbbreviation,
								  UmikoBot::get().getName(channel.guildId(), senderId),
								  UmikoBot::get().getName(channel.guildId(), receiverId)));
		embed.setColor(qrand() % 0xffffff);
		SEND_MESSAGE(embed);
	});
}

void CurrencyModule::steal(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	int amountInCents = args[1].toDouble() * 100;
	UserId victimId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[2]);
	UserId thiefId = message.author().id();

	// Just for ease of use
	UserCurrencyData& victimData = getUserCurrencyData(channel.guildId(), victimId);
	UserCurrencyData& thiefData = getUserCurrencyData(channel.guildId(), thiefId);
	const GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	
	if (thiefData.jailTimer)
	{
		SEND_MESSAGE(QString("You are in jail!\nYou will be released in **%1**!").arg(stringifyMilliseconds(thiefData.jailTimer->remainingTime())));
		return;
	}

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

	UmikoBot::get().getUser(victimId).then([this, message, channel, victimId, thiefId, amountInCents](const User& user)
	{
		// Just for ease of use
		UserCurrencyData& victimData = getUserCurrencyData(channel.guildId(), victimId);
		UserCurrencyData& thiefData = getUserCurrencyData(channel.guildId(), thiefId);
		const GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	
		if (user.bot())
		{
			SEND_MESSAGE("You cannot steal from us mighty bots!");
			return;
		}

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
			thiefData.amountStolenInCents += amountInCents;

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

			if (thiefData.jailTimer) delete thiefData.jailTimer;
			thiefData.jailTimer = new QTimer();
			thiefData.jailTimer->setSingleShot(true);
			thiefData.jailTimer->start(guildConfig.stealJailTimeMinutes * 60 * 1000);
			QObject::connect(thiefData.jailTimer, &QTimer::timeout, [this, channel, thiefId]()
			{
				delete getUserCurrencyData(channel.guildId(), thiefId).jailTimer;
				getUserCurrencyData(channel.guildId(), thiefId).jailTimer = nullptr;
			});

			Embed embed;
			embed.setTitle(":rotating_light: You Got Caught! :rotating_light:");
			embed.setColor(0xff0000);
			embed.setDescription(QString("%1 has been fined **%2 %3** and placed in jail.\n"
										 "%4 has been granted **%5 %3** as compensation.")
								 .arg(UmikoBot::get().getName(channel.guildId(), thiefId),
									  QString::number((amountInCents * guildConfig.stealFineAmount) / 100.0f),
									  guildConfig.currencyAbbreviation,
									  UmikoBot::get().getName(channel.guildId(), victimId),
									  QString::number((amountInCents * guildConfig.stealVictimBonus) / 100.0f)));
			SEND_MESSAGE(embed);
		}
	});
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
		UserId userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[2]);
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

	QList<UserCurrencyData> leaderboard = currencyData[channel.guildId()].values();
	if (min > (unsigned int) leaderboard.size())
	{
		SEND_MESSAGE("Not enough members to create the list!");
		return;
	}
	if (max > (unsigned int) leaderboard.size())
	{
		max = (unsigned int) leaderboard.size();
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
			if (max < (unsigned int) leaderboard.size())
			{
				max += 1;
			}

			continue;
		}

		description += QString("`%1`) **%2** - %3 %4\n").arg(QString::number(rank).rightJustified(numberOfDigits, ' '), name,
															 QString::number(leaderboard[i - 1].balanceInCents / 100.0f),
															 currencyConfigs[channel.guildId()].currencyAbbreviation);
		rank += 1;
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
	GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	const UserCurrencyData& userCurrencyData = getUserCurrencyData(channel.guildId(), message.author().id());

	if (userCurrencyData.jailTimer)
	{
		SEND_MESSAGE(QString("You are in jail!\nYou will be released in **%1**!").arg(stringifyMilliseconds(userCurrencyData.jailTimer->remainingTime())));
		return;
	}

	if (guildGambleData.currentUserId != 0)
	{
		SEND_MESSAGE(QString("Sorry, but this feature is currently being used by **%1**. Please try again later!")
					 .arg(UmikoBot::get().getName(channel.guildId(), guildGambleData.currentUserId)));
		return;
	}

	if (args.size() == 1)
	{
		guildGambleData.amountBetInCents = guildConfig.gambleDefaultAmountBet;
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

	if (userCurrencyData.balanceInCents - guildGambleData.amountBetInCents < guildConfig.maxDebt)
	{
		SEND_MESSAGE("You're too poor to be gambling that much!");
		return;
	}

	guildGambleData.currentUserId = message.author().id();

	// Timeout for a response that takes too long
	if (guildGambleData.idleTimeoutTimer) delete guildGambleData.idleTimeoutTimer;
	guildGambleData.idleTimeoutTimer = new QTimer();
	guildGambleData.idleTimeoutTimer->setSingleShot(true);
	guildGambleData.idleTimeoutTimer->start(guildConfig.gambleTimeoutSeconds * 1000);
	QObject::connect(guildGambleData.idleTimeoutTimer, &QTimer::timeout, [this, message, channel]()
	{
		GuildGambleData& guildGambleData = gambleData[channel.guildId()];
		
		if (guildGambleData.currentUserId != 0)
		{
			SEND_MESSAGE(QString("%1's gamble session has timed out!")
						 .arg(UmikoBot::get().getName(channel.guildId(), guildGambleData.currentUserId)));
			guildGambleData.currentUserId = 0;
		}

		delete guildGambleData.idleTimeoutTimer;
		guildGambleData.idleTimeoutTimer = nullptr;
	});
	
	Embed embed;
	embed.setTitle(QString("%1 is Gambling").arg(UmikoBot::get().getName(channel.guildId(), guildGambleData.currentUserId)));
	embed.setDescription(QString("All you need to do to win is to correctly guess a number between **1** and **5**!\n"
								 "Your Bet: **%1 %2**\n"
								 "Potential Winnings: **%3 %2**\n").arg(QString::number(guildGambleData.amountBetInCents / 100.0f),
																		guildConfig.currencyAbbreviation,
																		QString::number(guildGambleData.amountBetInCents * 4 / 100.0f)));
	embed.setColor(qrand() % 0xffffff);
	SEND_MESSAGE(embed);
}

void CurrencyModule::bribe(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	UserCurrencyData& userCurrencyData = getUserCurrencyData(channel.guildId(), message.author().id());
	GuildCurrencyConfig guildConfig = currencyConfigs[channel.guildId()];
	int amountToBribeInCents = args[1].toDouble() * 100;

	if (!userCurrencyData.jailTimer)
	{
		SEND_MESSAGE(":police_officer: **HEY!** You're not in jail! :police_officer:");
		return;
	}

	if (userCurrencyData.hasUsedBribe)
	{
		SEND_MESSAGE(":police_officer: You've already failed to bribe me... *do you want me to extend your sentence again?* :police_officer:");
		return;
	}

	if (amountToBribeInCents < guildConfig.bribeMinAmountInCents)
	{
		SEND_MESSAGE(QString(":police_officer: Pfft! Such a measly bribe! :police_officer: \n"
							 "I don't accept anything less than **%1 %2** for my time...")
					 .arg(QString::number(guildConfig.bribeMinAmountInCents / 100.0f), guildConfig.currencyAbbreviation));
		return;
	}

	if (amountToBribeInCents > guildConfig.bribeMaxAmountInCents)
	{
		SEND_MESSAGE(QString(":police_officer: Yikes! I can't accept more than **%1 %2**, too much risk!")
					 .arg(QString::number(guildConfig.bribeMaxAmountInCents / 100.0f), guildConfig.currencyAbbreviation));
		return;
	}

	if (userCurrencyData.balanceInCents - amountToBribeInCents < guildConfig.maxDebt)
	{
		SEND_MESSAGE("You need to sort out your *financial situation* before you can try to bribe me!");
		return;
	}

	int bribeAmountRange = guildConfig.bribeMaxAmountInCents - guildConfig.bribeMinAmountInCents;
	double bribeSuccessRange = guildConfig.bribeMaxSuccessChance - guildConfig.bribeMinSuccessChance;
	// amountOffset gives a 0 to 1 range of how much was bribed with (compared to min and max bribes)
	// successChance maps that 0 to 1 range between the min and max bribe success chances
	double amountOffset = (double) (amountToBribeInCents - guildConfig.bribeMinAmountInCents) / bribeAmountRange;
	double successChance = guildConfig.bribeMinSuccessChance + (amountOffset * bribeSuccessRange);

	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::discrete_distribution<> distribution { { 1 - successChance, successChance } };

	if (distribution(prng))
	{
		userCurrencyData.balanceInCents -= amountToBribeInCents;
		userCurrencyData.amountSpentOnBribingInCents += amountToBribeInCents;
		delete userCurrencyData.jailTimer;
		userCurrencyData.jailTimer = nullptr;

		SEND_MESSAGE(QString(":unlock: Thanks for that **BRIBE**!!! :unlock:\nI have freed you from jail and taken **%1 %2** from your wallet")
					 .arg(QString::number(amountToBribeInCents / 100.0f), guildConfig.currencyAbbreviation));;
	}
	else
	{
		userCurrencyData.hasUsedBribe = true;
		int newRemainingTime = userCurrencyData.jailTimer->remainingTime() + (guildConfig.bribeExtraJailTimeMinutes * 60 * 1000);
		userCurrencyData.jailTimer->start(newRemainingTime);

		SEND_MESSAGE(QString(":police_officer: Your bribes don't affect my loyalty! :police_officer:\n"
							 "Enjoy rotting in jail for another **%1 minutes**!\n"
							 "You will be released in **%2**!")
					 .arg(QString::number(guildConfig.bribeExtraJailTimeMinutes), stringifyMilliseconds(userCurrencyData.jailTimer->remainingTime())));
	}
}

void CurrencyModule::claim(const Message& message, const Channel& channel)
{
	GuildCurrencyConfig& guildConfig = currencyConfigs[channel.guildId()];
	UserCurrencyData& userCurrencyData = getUserCurrencyData(channel.guildId(), message.author().id());
	Embed embed;
	embed.setColor(qrand() % 0xffffff);;
	
	if (guildConfig.randomGiveawayDone)
	{
		embed.setTitle("Freebie Already Claimed");
		embed.setDescription(QString("Sorry, today's freebie has been claimed by **%1** :cry:\n\n"
									 "But you can always try again tomorrow!")
							 .arg(UmikoBot::get().getName(channel.guildId(), guildConfig.randomGiveawayClaimer)));
	}
	else if (guildConfig.randomGiveawayInProgress)
	{
		if (userCurrencyData.jailTimer)
		{
			SEND_MESSAGE("You are in jail!");
			return;
		}
		
		userCurrencyData.balanceInCents += guildConfig.randomGiveawayReward;
		userCurrencyData.numberOfGiveawaysClaimed += 1;
		guildConfig.randomGiveawayDone = true;
		guildConfig.randomGiveawayInProgress = false;
		guildConfig.randomGiveawayClaimer = message.author().id();
		
		embed.setTitle("Congratulations!");
		embed.setColor(0x00ff00);
		embed.setDescription(QString(":drum: And today's freebie goes to **%1**!\n\n"
									 "Congratulations! You just got **%2 %3**!").arg(UmikoBot::get().getName(channel.guildId(), message.author().id()),
																					 QString::number(guildConfig.randomGiveawayReward / 100.0f),
																					 guildConfig.currencyAbbreviation));
	}
	else
	{
		embed.setTitle("Not Yet!");
		embed.setDescription("**BRUH**, yOu CaN't JuSt GeT fReE sTuFf aNyTiMe!");
	}

	SEND_MESSAGE(embed);
}

void CurrencyModule::setCurrencyName(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].currencyName = args[1];
		currencyConfigs[channel.guildId()].currencyAbbreviation = args[2];
	}

	SEND_MESSAGE(QString("Currency name set to **%1** and abbreviation set to **%2**").arg(currencyConfigs[channel.guildId()].currencyName,
																						   currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setMaxDebt(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].maxDebt = args[1].toDouble() * 100;
	}
	
	SEND_MESSAGE(QString("Maximum debt set to **%1 %2**").arg(QString::number(currencyConfigs[channel.guildId()].maxDebt / 100.0f),
															  currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setDailyReward(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].rewardForDaily = args[1].toDouble() * 100;
	}
	
	SEND_MESSAGE(QString("Reward for daily set to **%1 %2**").arg(QString::number(currencyConfigs[channel.guildId()].rewardForDaily / 100.0f),
																  currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setDailyStreakBonus(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].dailyStreakBonus = args[1].toDouble() * 100;
	}

	SEND_MESSAGE(QString("Bonus for daily streak set to **%1 %2**").arg(QString::number(currencyConfigs[channel.guildId()].dailyStreakBonus / 100.0f),
																		currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setDailyStreakBonusPeriod(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].dailyStreakBonusPeriod = args[1].toInt();
	}

	SEND_MESSAGE(QString("Period for daily streak bonus set to **%1 days**")
				 .arg(QString::number(currencyConfigs[channel.guildId()].dailyStreakBonusPeriod)));
}

void CurrencyModule::setRandomGiveawayChance(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].randomGiveawayChance = args[1].toDouble();
	}

	SEND_MESSAGE(QString("Random giveaway chance set to **%1**").arg(QString::number(currencyConfigs[channel.guildId()].randomGiveawayChance)));
}

void CurrencyModule::setRandomGiveawayReward(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].randomGiveawayReward = args[1].toDouble() * 100;
	}

	SEND_MESSAGE(QString("Random giveaway reward set to **%1 %2**").arg(QString::number(currencyConfigs[channel.guildId()].randomGiveawayReward / 100.0f),
																		currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setStealSuccessChance(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	double newChance = args[1].toDouble();
	
	if (args.size() > 1)
	{
		if (newChance > 1.0)
		{
			SEND_MESSAGE("The new steal success chance must be between 0 and 1!");
			return;
		}
		
		currencyConfigs[channel.guildId()].stealSuccessBaseChance = newChance;
	}
	
	SEND_MESSAGE(QString("Steal success chance set to **%1**").arg(QString::number(newChance)));
}

void CurrencyModule::setStealFine(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].stealFineAmount = args[1].toDouble();
	}
	
	SEND_MESSAGE(QString("Steal fine portion set to **%1**").arg(QString::number(currencyConfigs[channel.guildId()].stealFineAmount)));
}

void CurrencyModule::setStealVictimBonus(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].stealVictimBonus = args[1].toDouble();
	}
	
	SEND_MESSAGE(QString("Steal victim bonus portion set to **%1**").arg(QString::number(currencyConfigs[channel.guildId()].stealVictimBonus)));;
}

void CurrencyModule::setStealJailTime(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].stealJailTimeMinutes = args[1].toUInt();
	}
	
	SEND_MESSAGE(QString("Jail time for a failed steal set to **%1 minutes**")
				 .arg(QString::number(currencyConfigs[channel.guildId()].stealJailTimeMinutes)));
}

void CurrencyModule::setBribeSuccessChance(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		if (args[1].toDouble() > 1.0 || args[2].toDouble() > 1.0)
		{
			SEND_MESSAGE("Chances must be between 0 and 1!");
			return;
		}
		if (args[1].toDouble() > args[2].toDouble())
		{
			SEND_MESSAGE("Maximum value must be larger than the minimum!");
			return;
		}

		currencyConfigs[channel.guildId()].bribeMinSuccessChance = args[1].toDouble();
		currencyConfigs[channel.guildId()].bribeMaxSuccessChance = args[2].toDouble();
	}
	
	SEND_MESSAGE(QString("Bribe success chance ranges from **%1** to **%2**")
				 .arg(QString::number(currencyConfigs[channel.guildId()].bribeMinSuccessChance),
					  QString::number(currencyConfigs[channel.guildId()].bribeMaxSuccessChance)));
}

void CurrencyModule::setBribeAmount(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		if (args[1].toDouble() > args[2].toDouble())
		{
			SEND_MESSAGE("Maximum value must be larger than the minimum!");
			return;
		}

		currencyConfigs[channel.guildId()].bribeMinAmountInCents = args[1].toDouble() * 100;
		currencyConfigs[channel.guildId()].bribeMaxAmountInCents = args[2].toDouble() * 100;
	}
	
	SEND_MESSAGE(QString("Accepted bribes range from **%1 %3** to **%2 %3**")
				 .arg(QString::number(currencyConfigs[channel.guildId()].bribeMinAmountInCents / 100.0f),
					  QString::number(currencyConfigs[channel.guildId()].bribeMaxAmountInCents / 100.0f),
					  currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setBribeExtraJailTime(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].bribeExtraJailTimeMinutes = args[1].toUInt();
	}
	
	SEND_MESSAGE(QString("Extra jail time from failed bribe set to **%1 minutes**")
				 .arg(QString::number(currencyConfigs[channel.guildId()].bribeExtraJailTimeMinutes)));
}

void CurrencyModule::setGambleDefaultBet(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].gambleDefaultAmountBet = args[1].toDouble() * 100;
	}
	
	SEND_MESSAGE(QString("Default bet for gamble set to **%1 %2**").arg(QString::number(currencyConfigs[channel.guildId()].gambleDefaultAmountBet / 100.0f),
																		currencyConfigs[channel.guildId()].currencyAbbreviation));
}

void CurrencyModule::setGambleTimeout(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args.size() > 1)
	{
		currencyConfigs[channel.guildId()].gambleTimeoutSeconds = args[1].toUInt();
	}
	
	SEND_MESSAGE(QString("Gamble timeout set to **%1 seconds**").arg(currencyConfigs[channel.guildId()].gambleTimeoutSeconds));
}
