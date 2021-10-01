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
	printf("Stopping bot...\n")
}

void UmikoBot::save()
{
}

void UmikoBot::load()
{
}

void UmikoBot::umikoOnReady()
{
}

void UmikoBot::umikoOnDisconnect()
{
}

void UmikoBot::umikoOnGuildCreate(const Discord::Guild& guild)
{
}

void UmikoBot::umikoOnGuildUpdate(const Discord::Guild& guild)
{
}

void UmikoBot::umikoOnGuildRoleUpdate(snowflake_t guildId, const Discord::Role& role)
{
}

void UmikoBot::umikoOnGuildRoleDelete(snowflake_t guildId, snowflake_t roleId)
{
}

void UmikoBot::umikoOnGuildMemberAdd(const Discord::GuildMember& member, snowflake_t guildId)
{
}

void UmikoBot::umikoOnGuildMemberUpdate(snowflake_t guildId, const QList<snowflake_t>& roles, const Discord::User& user, const QString& nickname)
{
}

void UmikoBot::umikoOnGuildMemberRemove(snowflake_t guildId, const Discord::User& user)
{
}

void UmikoBot::umikoOnMessageCreate(const Discord::Message& message)
{
}
