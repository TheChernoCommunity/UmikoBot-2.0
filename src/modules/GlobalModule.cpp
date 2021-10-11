#include "GlobalModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <Discord/Objects/Embed.h>
#include <QFile>

using namespace Discord;

GlobalModule::GlobalModule()
	: Module("Global")
{
	registerCommand(Commands::Help, "help" OPTIONAL(IDENTIFIER), CommandPermission::User, CALLBACK(help));
	registerCommand(Commands::Echo, "echo" TEXT, CommandPermission::User, CALLBACK(echo));
	registerCommand(Commands::SetPrefix, "set-prefix" IDENTIFIER, CommandPermission::Moderator, CALLBACK(setPrefix));
	registerCommand(Commands::Enable, "enable" SPACE "(module|command)" IDENTIFIER, CommandPermission::Moderator, CALLBACK(enable));
	registerCommand(Commands::Disable, "disable" SPACE "(module|command)" IDENTIFIER, CommandPermission::Moderator, CALLBACK(disable));
}

GlobalModule::~GlobalModule()
{
}

void GlobalModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject commandsEnabledObject {};

	for (Module* module : UmikoBot::get().getModules())
	{
		for (const Command& command : module->getCommands())
		{
			commandsEnabledObject[command.name] = command.enabled;;
		}
	}

	mainObject["commandsEnabled"] = commandsEnabledObject;
}

void GlobalModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject commandsEnabledObject = mainObject["commandsEnabled"].toObject();

	for (Module* module : UmikoBot::get().getModules())
	{
		for (Command& command : module->getCommands())
		{
			command.enabled = commandsEnabledObject[command.name].toBool(true);
		}
	}
}

void GlobalModule::help(const Message& message, const Channel& channel)
{
	::Permissions::contains(channel.guildId(), message.author().id(), CommandPermission::Moderator, [this, message, channel](bool result)
	{
		QString output = "";
		QStringList args = message.content().split(QRegularExpression(SPACE));
		QString prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;
	
		if (args.size() == 1)
		{
			for (Module* module : UmikoBot::get().getModules())
			{
				QString adminCommandsOutput = "";
				output += "**" + module->getName() + "**\n";
			
				for (const Command& command : module->getCommands())
				{
					bool adminRequired = CommandInfo::adminRequired[(unsigned int) command.id];
					if (adminRequired && !result)
					{
						continue;
					}
				
					QString description = CommandInfo::briefDescription[(unsigned int) command.id];
					if (description == "")
					{
						description = "*No description found*";
					}

					// Admin commands are displayed at the end
					QString& outputToUse = adminRequired ? adminCommandsOutput : output;
					outputToUse += QString("`%1%2` - %3\n").arg(prefix, command.name, description);
				}

				if (adminCommandsOutput != "")
				{
					output += "\n" + adminCommandsOutput;
				}
				output += "\n";
			}
		}
		else
		{
			output = commandHelp(args[1], prefix);

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
	});
}

void GlobalModule::echo(const Message& message, const Channel& channel)
{
	QString restOfMessage = message.content().mid(message.content().indexOf(QRegularExpression("\\s")));
	SEND_MESSAGE(restOfMessage);
}

void GlobalModule::setPrefix(const Message& message, const Channel& channel)
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

void GlobalModule::enable(const Message& message, const Channel& channel)
{
	enableDisableImpl(message, channel, true);
}

void GlobalModule::disable(const Message& message, const Channel& channel)
{
	enableDisableImpl(message, channel, false);
}

QString GlobalModule::commandHelp(const QString& request, const QString& prefix)
{
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

				QString output = "";
				output += "**Command name:** `" + command.name + "`\n\n";
				output += description + "\n" + additionalInfo + "\n";
				if (additionalInfo != "")
				{
					output += "\n";
				}
				output += "**Usage:**`" + prefix + usage + "`";

				return output;
			}
		}
	}

	return "";
}

bool canBeDisabled(const Command& command)
{
	return command.name != "help" && command.name != "enable" && command.name != "disable";
}

void GlobalModule::enableDisableImpl(const Message& message, const Channel& channel, bool enable)
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
