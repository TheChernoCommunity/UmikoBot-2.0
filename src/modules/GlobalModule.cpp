#include "GlobalModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <Discord/Objects/Embed.h>

using namespace Discord;

GlobalModule::GlobalModule()
	: Module("Global")
{
	registerCommand(Commands::Help, "help", CommandPermission::User, help);
	registerCommand(Commands::Echo, "echo\\s.+", CommandPermission::User, echo);
	registerCommand(Commands::SetPrefix, "set-prefix\\s\\S+", CommandPermission::Moderator, setPrefix);
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

void GlobalModule::setPrefix(const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression("\\s"));
	QString& prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;

	if (prefix == args[1])
	{
		SEND_MESSAGE("That is already the prefix!");
	}
	else
	{
		prefix = args[1];
		SEND_MESSAGE(QString("Changed prefix to '%1'").arg(prefix));
	}
}
