#pragma once

#include <Discord/Client.h>
#include <Discord/Objects/Guild.h>
#include <Discord/Objects/GuildMember.h>

#include "core/Data.h"
#include "modules/Module.h"

class UmikoBot : public Discord::Client
{
public:
	static UmikoBot& get();

	UmikoBot(const UmikoBot&) = delete;
	void operator=(const UmikoBot&) = delete;
	~UmikoBot();

	bool isOwner(snowflake_t guildId, snowflake_t userId);
	const QList<Discord::Role>& getRoles(snowflake_t guildId);
	const QList<Module*>& getModules() const { return modules; }
	QMap<snowflake_t, GuildData>& getGuildData() { return guildData; }

	const QString& getNickname(snowflake_t guildId, snowflake_t userId);
	const QString& getUsername(snowflake_t guildId, snowflake_t userId);
	const QString& getName(snowflake_t guildId, snowflake_t userId);
	Discord::Promise<QString>& getAvatar(snowflake_t guildId, snowflake_t userId);
	
private:
	UmikoBot(QObject* parent = nullptr);

	void save();
	void saveGuildData();
	void load();
	void loadGuildData();
	
	void initialiseGuilds(snowflake_t afterId = 0);
	void initialiseGuildMembers(snowflake_t guildId, snowflake_t afterId = 0);

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
	QMap<snowflake_t /* guildId */, GuildData> guildData;

	constexpr inline static const char* SETTINGS_LOCATION = "configs/settings.json";
};
