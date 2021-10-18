#include "FunModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <random>

using namespace Discord;

FunModule::FunModule()
	: Module("Fun")
{
	namespace CP = CommandPermission;
	registerCommand(Commands::Roll, "roll" INTEGER OPTIONAL(INTEGER), CP::User, CALLBACK(roll));
}

void FunModule::roll(const Message& message, const Channel& channel)
{
	(void) channel;
	QStringList args = message.content().split(QRegularExpression(SPACE));
	int min = 0;
	int max = 0;
	bool okMin = true;
	bool okMax = true;

	if (args.size() == 2)
	{
		max = args[1].toInt(&okMax);
	}
	else if (args.size() == 3)
	{
		min = args[1].toInt(&okMin);
		max = args[2].toInt(&okMax);
	}

	if (!okMin || !okMax)
	{
		SEND_MESSAGE("One of your bounds is invalid!");
		return;
	}

	if (min > max)
	{
		std::swap(min, max);
	}

	std::random_device randomDevice;
	std::mt19937 prng { randomDevice() };
	std::uniform_int_distribution<> distribution(min, max);
	SEND_MESSAGE(QString("My value was **%1**!").arg(distribution(prng)));
}
