#pragma once

#include <Discord/Client.h>
#include <Discord/Objects/Guild.h>
#include <Discord/Objects/GuildMember.h>

#include "core/Module.h"

class UmikoBot : public Discord::Client
{
public:
	static UmikoBot& get();

	UmikoBot(const UmikoBot&) = delete;
	void operator=(const UmikoBot&) = delete;
	~UmikoBot();

private:
	UmikoBot(QObject* parent = nullptr);

	void save();
	void load();

private slots:
	void umikoOnReady();
	void umikoOnDisconnect();

	void umikoOnGuildCreate(const Discord::Guild& guild);
	void umikoOnGuildUpdate(const Discord::Guild& guild);

	void umikoOnGuildRoleUpdate(snowflake_t guildId, const Discord::Role& role);
	void umikoOnGuildRoleDelete(snowflake_t guildId, snowflake_t roleId);

	void umikoOnGuildMemberAdd(const Discord::GuildMember& member, snowflake_t guildId);
	void umikoOnGuildMemberUpdate(snowflake_t guildId, const QList<snowflake_t>& roles, const Discord::User& user, const QString& nickname);
	void umikoOnGuildMemberRemove(snowflake_t guildId, const Discord::User& user);

	void umikoOnMessageCreate(const Discord::Message& message);
	
private:
	QTimer saveTimer;

	QList<Module*> modules;
};
