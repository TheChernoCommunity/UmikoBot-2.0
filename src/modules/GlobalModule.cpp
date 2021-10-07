#include "GlobalModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <Discord/Objects/Embed.h>

using namespace Discord;

GlobalModule::GlobalModule()
	: Module("Global")
{
	registerCommand(Commands::Help, "help" OPTIONAL(IDENTIFIER), CommandPermission::User, help);
	registerCommand(Commands::Echo, "echo" TEXT, CommandPermission::User, echo);
	registerCommand(Commands::SetPrefix, "set-prefix" IDENTIFIER, CommandPermission::Moderator, setPrefix);
	registerCommand(Commands::Enable, "enable" SPACE "(module|command)" IDENTIFIER, CommandPermission::Moderator, enable);
	registerCommand(Commands::Disable, "disable" SPACE "(module|command)" IDENTIFIER, CommandPermission::Moderator, disable);
}

GlobalModule::~GlobalModule()
{
}

void GlobalModule::help(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	QString prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;
	QString output = "";
	
	// TODO(fkp): Separate admin commands
	if (args.size() == 1)
	{
		for (Module* module : UmikoBot::get().getModules())
		{
			output += "**" + module->getName() + "**\n";
			
			for (const Command& command : module->getCommands())
			{
				QString description = CommandInfo::briefDescription[(unsigned int) command.id];
				if (description == "")
				{
					description = "*No description found*";
				}
				
				output += QString("`%1%2` - %3\n").arg(prefix, command.name, description);
			}

			output += "\n";
		}
	}
	else
	{
		const QString& request = args[1];
		for (Module* module : UmikoBot::get().getModules())
		{
			for (const Command& command : module->getCommands())
			{
				if (command.name == request)
				{
					const QString& description = CommandInfo::briefDescription[(unsigned int) command.id];
					const QString& usage = CommandInfo::usage[(unsigned int) command.id];
					const QString& additionalInfo = CommandInfo::additionalInfo[(unsigned int) command.id];
					bool adminRequired = CommandInfo::adminRequired[(unsigned int) command.id];

					output += "**Command name:** `" + command.name + "`\n\n";
					output += description + "\n" + additionalInfo + "\n";
					if (additionalInfo != "")
					{
						output += "\n";
					}
					output += "**Usage:**`" + prefix + usage + "`";
				}
			}
		}

		if (output == "")
		{
			SEND_MESSAGE("I could not find that command!");
			return;
		}
	}

	Embed embed {};
	embed.setColor(qrand() % 0xffffff);
	embed.setTitle("Help");
	embed.setDescription(output);
	SEND_MESSAGE(embed);
}

void GlobalModule::echo(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	QString restOfMessage = message.content().mid(message.content().indexOf(QRegularExpression("\\s")));
	SEND_MESSAGE(restOfMessage);
}

void GlobalModule::setPrefix(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
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

void GlobalModule::enable(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	enableDisableImpl(module, message, channel, true);
}

void GlobalModule::disable(Module* module, const Discord::Message& message, const Discord::Channel& channel)
{
	enableDisableImpl(module, message, channel, false);
}

bool canBeDisabled(const Command& command)
{
	return command.name != "help" && command.name != "enable" && command.name != "disable";
}

void GlobalModule::enableDisableImpl(Module* module, const Discord::Message& message, const Discord::Channel& channel, bool enable)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	QString output = "";

	for (Module* module : UmikoBot::get().getModules())
	{
		if (args[1] == "module" && module->getName() != args[2])
		{
			continue;
		}

		for (Command& command : module->getCommands())
		{
			if (args[1] == "command" && command.name != args[2])
			{
				continue;
			}

			if (!enable && !canBeDisabled(command))
			{
				output += "You can't disable command `" + command.name + "`\n";
				continue;
			}
		
			command.enabled = enable;
			output += QString("%1 command `%2`\n").arg(enable ? "Enabled" : "Disabled", command.name);		
		}
	}

	if (output == "")
	{
		output = "Could not find that " + args[1] + "!";
	}

	SEND_MESSAGE(output);
}
