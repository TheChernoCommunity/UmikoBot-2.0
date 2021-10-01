#include "GlobalModule.h"
#include "UmikoBot.h"

GlobalModule::GlobalModule()
	: Module("currency")
{
	registerCommand(Commands::Help, "help", help);
}

GlobalModule::~GlobalModule()
{
}

void GlobalModule::help(const Discord::Message& message, const Discord::Channel& channel)
{
	UmikoBot::get().createMessage(message.channelId(), "Help message!");
}
