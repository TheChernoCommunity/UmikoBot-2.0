#pragma once

#include <Discord/Client.h>
#include <Discord/Objects/Guild.h>
#include <Discord/Objects/GuildMember.h>

#include "core/Core.h"
#include "core/Data.h"
#include "modules/Module.h"

class UmikoBot : public Discord::Client
{
public:
	static UmikoBot& get();

	UmikoBot(const UmikoBot&) = delete;
	void operator=(const UmikoBot&) = delete;
	~UmikoBot();

	bool isOwner(GuildId guildId, UserId userId);
	const QList<Discord::Role>& getRoles(GuildId guildId);
	const QList<Module*>& getModules() const { return modules; }
	QMap<GuildId, GuildData>& getGuildData() { return guildData; }

	const QString& getNickname(GuildId guildId, UserId userId);
	const QString& getUsername(GuildId guildId, UserId userId);
	const QString& getName(GuildId guildId, UserId userId);
	Discord::Promise<QString>& getAvatar(GuildId guildId, UserId userId);

	UserId getUserIdFromArgument(GuildId guildId, const QString& argument);
	UserId getUserIdFromMention(GuildId guildId, const QString& mention);
	Discord::Promise<Discord::Channel>& getChannelFromArgument(GuildId guildId, const QString& argument);
	ChannelId getChannelIdFromArgument(const QList<Discord::Channel>& channels, const QString& argument);

private:
	UmikoBot(QObject* parent = nullptr);

	void save();
	void saveGuildData();
	void load();
	void loadGuildData();
	
	void initialiseGuilds(GuildId afterId = 0);
	void initialiseGuildMembers(GuildId guildId, UserId afterId = 0);

private slots:
	void umikoOnReady();
	void umikoOnDisconnect();

	void umikoOnGuildCreate(const Discord::Guild& guild);
	void umikoOnGuildUpdate(const Discord::Guild& guild);

	void umikoOnGuildRoleUpdate(GuildId guildId, const Discord::Role& role);
	void umikoOnGuildRoleDelete(GuildId guildId, RoleId roleId);

	void umikoOnGuildMemberAdd(const Discord::GuildMember& member, GuildId guildId);
	void umikoOnGuildMemberUpdate(GuildId guildId, const QList<RoleId>& roles, const Discord::User& user, const QString& nickname);
	void umikoOnGuildMemberRemove(GuildId guildId, const Discord::User& user);

	void umikoOnMessageCreate(const Discord::Message& message);
	
public:
	QMap<GuildId, ChannelId> primaryChannels;

private:
	QTimer saveTimer;

	QList<Module*> modules;
	QMap<GuildId, GuildData> guildData;

	constexpr inline static const char* SETTINGS_LOCATION = "configs/settings.json";
};
