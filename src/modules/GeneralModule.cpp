#include "GeneralModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

#include <Discord/Objects/Embed.h>
#include <QFile>
#include <QJsonArray>

using namespace Discord;

GeneralModule::GeneralModule()
	: Module("General")
{
	registerCommand(Commands::Help, "help" OPTIONAL(IDENTIFIER), CommandPermission::User, CALLBACK(help));
	registerCommand(Commands::Echo, "echo" TEXT, CommandPermission::Moderator, CALLBACK(echo));
	registerCommand(Commands::Status, "status" OPTIONAL(IDENTIFIER), CommandPermission::User, CALLBACK(status));
	registerCommand(Commands::SetPrefix, "set-prefix" OPTIONAL(IDENTIFIER), CommandPermission::Moderator, CALLBACK(setPrefix));
	registerCommand(Commands::Enable, "enable" SPACE "(channel|module|command)" IDENTIFIER, CommandPermission::Moderator, CALLBACK(enable));
	registerCommand(Commands::Disable, "disable" SPACE "(channel|module|command)" IDENTIFIER, CommandPermission::Moderator, CALLBACK(disable));
	registerCommand(Commands::SetPrimaryChannel, "set-primary-channel" OPTIONAL(CHANNEL), CommandPermission::Moderator, CALLBACK(setPrimaryChannel));
}

void GeneralModule::onSave(QJsonObject& mainObject) const
{
	QJsonObject commandsEnabledObject {};
	QJsonObject primaryChannelsObject {};
	QJsonObject channelsEnabledObject {};

	for (Module* module : UmikoBot::get().getModules())
	{
		for (const Command& command : module->getCommands())
		{
			commandsEnabledObject[command.name] = command.enabled;;
		}
	}

	for (GuildId guildId : UmikoBot::get().primaryChannels.keys())
	{
		primaryChannelsObject[QString::number(guildId)] = QString::number(UmikoBot::get().primaryChannels[guildId]);
	}

	for (GuildId guildId : UmikoBot::get().channelsEnabled.keys())
	{
		QJsonArray channelsEnabledArray;
		for (ChannelId channelId : UmikoBot::get().channelsEnabled[guildId])
		{
			channelsEnabledArray.append(QString::number(channelId));
		}

		channelsEnabledObject[QString::number(guildId)] = channelsEnabledArray;
	}
	
	mainObject["commandsEnabled"] = commandsEnabledObject;
	mainObject["primaryChannels"] = primaryChannelsObject;
	mainObject["channelsEnabled"] = channelsEnabledObject;
}

void GeneralModule::onLoad(const QJsonObject& mainObject)
{
	QJsonObject commandsEnabledObject = mainObject["commandsEnabled"].toObject();
	for (Module* module : UmikoBot::get().getModules())
	{
		for (Command& command : module->getCommands())
		{
			command.enabled = commandsEnabledObject[command.name].toBool(true);
		}
	}

	QJsonObject primaryChannelsObject = mainObject["primaryChannels"].toObject();
	for (const QString& guildIdString : primaryChannelsObject.keys())
	{
		UmikoBot::get().primaryChannels[guildIdString.toULongLong()] = primaryChannelsObject[guildIdString].toString().toULongLong();
	}

	QJsonObject channelsEnabledObject = mainObject["channelsEnabled"].toObject();
	for (const QString& guildIdString : channelsEnabledObject.keys())
	{
		QJsonArray channelsEnabledArray = channelsEnabledObject[guildIdString].toArray();
		QSet<ChannelId> channelsEnabledSet;
		for (const QJsonValue& channelId : channelsEnabledArray)
		{
			channelsEnabledSet.insert(channelId.toString().toULongLong());
		}

		UmikoBot::get().channelsEnabled[guildIdString.toULongLong()] = channelsEnabledSet;
	}
}

void GeneralModule::help(const Message& message, const Channel& channel)
{
	containsPermission(channel.guildId(), message.author().id(), CommandPermission::Moderator, [this, message, channel](bool result)
	{
		QString output = "";
		QStringList args = message.content().split(QRegularExpression(SPACE));
		QString prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;
	
		if (args.size() == 1)
		{
			for (Module* module : UmikoBot::get().getModules())
			{
				QString userCommandsOutput = "";
				QString adminCommandsOutput = "";

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
					QString& outputToUse = adminRequired ? adminCommandsOutput : userCommandsOutput;
					outputToUse += QString("`%1%2` - %3\n").arg(prefix, command.name, description);
				}

				if (userCommandsOutput == "" && adminCommandsOutput == "")
				{
					continue;
				}

				output += "**" + module->getName() + "**\n";					

				if (userCommandsOutput != "")
				{
					output += userCommandsOutput + "\n";
				}
				if (adminCommandsOutput != "")
				{
					output += adminCommandsOutput + "\n";
				}
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

void GeneralModule::echo(const Message& message, const Channel& channel)
{
	QString restOfMessage = message.content().mid(message.content().indexOf(QRegularExpression("\\s")));
	SEND_MESSAGE(restOfMessage);
}

void GeneralModule::status(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	UserId userId = message.author().id();

	if (args.size() == 2)
	{
		userId = UmikoBot::get().getUserIdFromArgument(channel.guildId(), args[1]);
		if (!userId)
		{
			SEND_MESSAGE("Could not find user!");
			return;
		}
	}

	UmikoBot::get().getAvatar(channel.guildId(), userId).then([this, message, channel, userId](const QString& icon)
	{
		QString output = "";
		for (Module* module : UmikoBot::get().getModules())
		{
			module->onStatus(output, channel.guildId(), userId);
		}

		Embed embed;
		embed.setTitle("Status");
		embed.setAuthor(EmbedAuthor { UmikoBot::get().getName(channel.guildId(), userId), "", icon });
		embed.setColor(qrand() % 0xffffff);
		embed.setDescription(output);
		SEND_MESSAGE(embed);
	});
}

void GeneralModule::setPrefix(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	QString& prefix = UmikoBot::get().getGuildData()[channel.guildId()].prefix;

	if (args.size() == 1)
	{
		SEND_MESSAGE(QString("Current prefix is **%1**").arg(prefix));
	}
	else
	{
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
}

void GeneralModule::enable(const Message& message, const Channel& channel)
{
	enableDisableImpl(message, channel, true);
}

void GeneralModule::disable(const Message& message, const Channel& channel)
{
	enableDisableImpl(message, channel, false);
}

void GeneralModule::setPrimaryChannel(const Message& message, const Channel& channel)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));
	
	if (args.size() > 1)
	{
		UmikoBot::get().getChannelFromArgument(channel.guildId(), args[1]).then([this, message, channel](const Channel& mentionedChannel)
		{
			UmikoBot::get().primaryChannels[channel.guildId()] = mentionedChannel.id();
			SEND_MESSAGE(QString("Primary channel set to <#%1>").arg(QString::number(UmikoBot::get().primaryChannels[channel.guildId()])));
		})
		.otherwise([this, message]()
		{
			SEND_MESSAGE("Could not find channel!");
		});
	}
	else
	{
		if (UmikoBot::get().primaryChannels[channel.guildId()])
		{
			SEND_MESSAGE(QString("Primary channel is <#%1>").arg(QString::number(UmikoBot::get().primaryChannels[channel.guildId()])));
		}
		else
		{
			SEND_MESSAGE("No primary channel has been set!");
		}
	}
}

QString GeneralModule::commandHelp(const QString& request, const QString& prefix)
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

void GeneralModule::enableDisableImpl(const Message& message, const Channel& channel, bool enable)
{
	QStringList args = message.content().split(QRegularExpression(SPACE));

	if (args[1] == "channel")
	{
		UmikoBot::get().getGuildChannels(channel.guildId()).then([message, channel, enable, args](const QList<Channel>& channels)
		{
			ChannelId channelId = UmikoBot::get().getChannelIdFromArgument(channels, args[2]);
			if (!channelId)
			{
				SEND_MESSAGE("Could not find that channel!");
				return;
			}

			QSet<ChannelId>& guildEnabledChannels = UmikoBot::get().channelsEnabled[channel.guildId()];
			if (enable)
			{
				guildEnabledChannels.insert(channelId);
			}
			else
			{
				guildEnabledChannels.remove(channelId);
			}

			SEND_MESSAGE(QString("%1 output in <#%2>").arg(enable ? "Enabled" : "Disabled", QString::number(channelId)));
		});
	}
	else if (args[1] == "module" || args[1] == "command")
	{
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
}
