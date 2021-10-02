#include "GlobalModule.h"
#include "UmikoBot.h"

#include <Discord/Objects/Embed.h>

using namespace Discord;

GlobalModule::GlobalModule()
	: Module("Global")
{
	registerCommand(Commands::Help, "help", help);
	registerCommand(Commands::Help, "echo .+", echo);
}

GlobalModule::~GlobalModule()
{
}

void GlobalModule::help(const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression("\\s"));
	QString prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;
	QString output = "";
	
	if (args.size() == 1)
	{
		for (Module* module : UmikoBot::get().getModules())
		{
			output += "**" + module->getName() + "**\n";
			
			for (const Command& command : module->getCommands())
			{
				output += QString("`%1%2` - %3\n").arg(prefix, command.name, "TODO: description");
			}

			output += "\n";
		}
	}

	Embed embed {};
	embed.setColor(qrand() % 0xffffff);
	embed.setTitle("Help");
	embed.setDescription(output);
	SEND_MESSAGE(embed);
}

void GlobalModule::echo(const Discord::Message& message, const Discord::Channel& channel)
{
	QString restOfMessage = message.content().mid(message.content().indexOf(QRegularExpression("\\s")));
	SEND_MESSAGE(restOfMessage);
}

