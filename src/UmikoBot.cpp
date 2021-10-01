#include <stdio.h>

#include "UmikoBot.h"

using namespace Discord;

UmikoBot& UmikoBot::get()
{
	static UmikoBot bot;
	return bot;
}

UmikoBot::UmikoBot(QObject* parent)
	: Client("umiko-bot", parent)
{
	printf("Starting bot...\n");

	load();
	saveTimer.setInterval(60 * 1000);
	QObject::connect(&saveTimer, &QTimer::timeout, [this]()
	{
		save();
	});

	// Event callback connections
	connect(this, &Client::onReady, this, &UmikoBot::umikoOnReady);
	connect(&getGatewaySocket(), &GatewaySocket::disconnected, this, &UmikoBot::umikoOnDisconnect);

	connect(this, &Client::onGuildCreate, this, &UmikoBot::umikoOnGuildCreate);
	connect(this, &Client::onGuildUpdate, this, &UmikoBot::umikoOnGuildUpdate);

	connect(this, &Client::onGuildRoleUpdate, this, &UmikoBot::umikoOnGuildRoleUpdate);
	connect(this, &Client::onGuildRoleDelete, this, &UmikoBot::umikoOnGuildRoleDelete);

	connect(this, &Client::onGuildMemberAdd, this, &UmikoBot::umikoOnGuildMemberAdd);
	connect(this, &Client::onGuildMemberUpdate, this, &UmikoBot::umikoOnGuildMemberUpdate);
	connect(this, &Client::onGuildMemberRemove, this, &UmikoBot::umikoOnGuildMemberRemove);
	connect(this, &Client::onMessageCreate, this, &UmikoBot::umikoOnMessageCreate);
}

UmikoBot::~UmikoBot()
{
	save();
	printf("Stopping bot...\n");
}

void UmikoBot::save()
{
}

void UmikoBot::load()
{
}

void UmikoBot::umikoOnReady()
{
	printf("Ready!\n");
}

void UmikoBot::umikoOnDisconnect()
{
}

void UmikoBot::umikoOnGuildCreate(const Guild& guild)
{
}

void UmikoBot::umikoOnGuildUpdate(const Guild& guild)
{
}

void UmikoBot::umikoOnGuildRoleUpdate(snowflake_t guildId, const Role& role)
{
}

void UmikoBot::umikoOnGuildRoleDelete(snowflake_t guildId, snowflake_t roleId)
{
}

void UmikoBot::umikoOnGuildMemberAdd(const GuildMember& member, snowflake_t guildId)
{
}

void UmikoBot::umikoOnGuildMemberUpdate(snowflake_t guildId, const QList<snowflake_t>& roles, const User& user, const QString& nickname)
{
}

void UmikoBot::umikoOnGuildMemberRemove(snowflake_t guildId, const User& user)
{
}

void UmikoBot::umikoOnMessageCreate(const Message& message)
{
	QString messageString = message.content();
	bool isCommand = false;
	
	for (Module* module : modules)
	{
		for (Command& command : module->getCommands())
		{
			if (!command.enabled)
			{
				continue;
			}
			
			if (command.regex.match(messageString).hasMatch())
			{
				isCommand = true;
				UmikoBot::get().getChannel(message.channelId()).then([this, message, command](const Channel& channel)
				{
					command.callback(message, channel);
				});
			}
		}
	}

	if (!isCommand)
	{
		for (Module* module : modules)
		{
			module->onMessage(message);
		}
	}
}
