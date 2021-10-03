#include <stdio.h>

#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDir>

#include "UmikoBot.h"
#include "core/Permissions.h"
#include "modules/GlobalModule.h"

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

	QDir().mkdir("configs");
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

	// Modules
	modules.push_back(new GlobalModule());
}

UmikoBot::~UmikoBot()
{
	save();
	printf("Stopping bot...\n");
}

void UmikoBot::save()
{
	saveGuildData();
}

void UmikoBot::saveGuildData()
{
	QJsonObject json;

	for (const GuildData& data : guildData.values())
	{
		json[QString::number(data.guildId)] = data.writeToObject();
	}

	QJsonDocument doc { json };
	QString result = doc.toJson(QJsonDocument::Indented);

	// Writes the JSON out to file
	QFile file { SETTINGS_LOCATION };
	if (!file.open(QIODevice::WriteOnly))
	{
		printf("Unable to open file for saving: \"%s\"\n", SETTINGS_LOCATION);
		return;
	}
	
	file.write(qPrintable(result));
	file.close();
}

void UmikoBot::load()
{
	loadGuildData();

	// Parses the full enum string into separate commands
	QStringList individialCommands = CommandInfo::enumFullString.split(",");
	for (unsigned int i = 0; i < (unsigned int) Commands::Count; i++)
	{
		CommandInfo::commandStrings[i] = individialCommands[i].trimmed();
	}

	// Loads the commands
	QFile file { "commands.json" };
	if (!file.open(QIODevice::ReadOnly))
	{
		printf("Failed to open commands.json for reading...\n");
		return;
	}

	QByteArray data = file.readAll();
	QJsonDocument doc = QJsonDocument::fromJson(data);
	QJsonObject json = doc.object();

	for (unsigned int i = 0; i < (unsigned int) Commands::Count; i++)
	{
		QJsonValue commandJson = json[CommandInfo::commandStrings[i]];

		// Would be undefined if the command has no description
		if (commandJson != QJsonValue::Undefined)
		{
			QJsonObject current = commandJson.toObject();
			
			CommandInfo::briefDescription[i] = current["brief"].toString();
			CommandInfo::usage[i] = current["usage"].toString();
			CommandInfo::additionalInfo[i] = current["additional"].toString();
			CommandInfo::adminRequired[i] = current["admin"].toBool();
		}
	}

	file.close();
}

void UmikoBot::loadGuildData()
{
	QFile file { SETTINGS_LOCATION };
	if (!file.open(QIODevice::ReadOnly))
	{
		printf("Unable to open file for loading: \"%s\"\n", SETTINGS_LOCATION);
		return;
	}

	QByteArray fileData = file.readAll();
	QJsonDocument doc = QJsonDocument::fromJson(fileData);
	QJsonObject json = doc.object();
	QStringList guildIds = json.keys();

	for (const QString& guildId : guildIds)
	{
		QJsonObject current = json.value(guildId).toObject();
		guildData[guildId.toULongLong()] = GuildData::createFromObject(guildId.toULongLong(), current);
	}

	file.close();
}

void UmikoBot::initialiseGuilds(snowflake_t afterId)
{
	constexpr snowflake_t LIMIT = 100;
	
	getCurrentUserGuilds(0, afterId, LIMIT).then([this](const QList<Guild>& guilds)
	{
		for (const Guild& guild : guilds)
		{
			getGuild(guild.id()).then([this](const Guild& guild)
			{
				guildData[guild.id()].ownerId = guild.ownerId();
			});

			getGuildRoles(guild.id()).then([this, guild](const QList<Role>& roles)
			{
				guildData[guild.id()].roles = roles;
			});

			initialiseGuildMembers(guild.id());
		}

		if (guilds.size() == LIMIT)
		{
			// More to come
			initialiseGuilds(guilds.back().id());
		}
		else
		{
			printf("Guild count: %d\n", guilds.size());
		}
	});
}

void UmikoBot::initialiseGuildMembers(snowflake_t guildId, snowflake_t afterId)
{
	constexpr snowflake_t LIMIT = 1000;
	
	listGuildMembers(guildId, LIMIT, afterId).then([this, guildId](const QList<GuildMember>& members)
	{
		for (const GuildMember& member : members)
		{
			guildData[guildId].userData[member.user().id()].username = member.user().username();
			guildData[guildId].userData[member.user().id()].nickname = member.nick();
		}

		if (members.size() == LIMIT)
		{
			// More to come
			initialiseGuildMembers(guildId, members.back().user().id());
		}
		
		printf("Guild %llu: %d members\n", guildId, members.size());
	});
}

bool UmikoBot::isOwner(snowflake_t guildId, snowflake_t userId)
{
	return guildData[guildId].ownerId == userId;
}

const QList<Discord::Role>& UmikoBot::getRoles(snowflake_t guildId)
{
	return guildData[guildId].roles;
}

void UmikoBot::umikoOnReady()
{
	printf("Ready!\n");
	initialiseGuilds();
}

void UmikoBot::umikoOnDisconnect()
{
}

void UmikoBot::umikoOnGuildCreate(const Guild& guild)
{
	guildData[guild.id()] = GuildData { guild.id() };
	initialiseGuilds(); // TODO(fkp): Only initialise a single guild?
}

void UmikoBot::umikoOnGuildUpdate(const Guild& guild)
{
	guildData[guild.id()].ownerId = guild.ownerId();
}

void UmikoBot::umikoOnGuildRoleUpdate(snowflake_t guildId, const Role& newRole)
{
	for (Role& role : guildData[guildId].roles)
	{
		if (role.id() == newRole.id())
		{
			role = newRole;
			return;
		}
	}

	guildData[guildId].roles.push_back(newRole);
}

void UmikoBot::umikoOnGuildRoleDelete(snowflake_t guildId, snowflake_t roleId)
{
	for (int i = 0; i < guildData[guildId].roles.size(); i++)
	{
		if (guildData[guildId].roles[i].id() == roleId)
		{
			guildData[guildId].roles.erase(guildData[guildId].roles.begin() + i);
			return;
		}
	}
}

void UmikoBot::umikoOnGuildMemberAdd(const GuildMember& member, snowflake_t guildId)
{
	guildData[guildId].userData[member.user().id()].username = member.user().username();
}

void UmikoBot::umikoOnGuildMemberUpdate(snowflake_t guildId, const QList<snowflake_t>& roles, const User& user, const QString& nickname)
{
	guildData[guildId].userData[user.id()].username = user.username();
	guildData[guildId].userData[user.id()].nickname = nickname;
}

void UmikoBot::umikoOnGuildMemberRemove(snowflake_t guildId, const User& user)
{
	for (auto it = guildData[guildId].userData.begin(); it != guildData[guildId].userData.end(); it++)
	{
		if (it.key() == user.id())
		{
			guildData[guildId].userData.erase(it);
			return;
		}
	}
}

void UmikoBot::umikoOnMessageCreate(const Message& message)
{
	getChannel(message.channelId()).then([this, message](const Channel& channel)
	{
		// messageString -> !status Name
		// prefix        -> !
		// fullCommand   ->  status Name
		// commandName   ->  status
		QString messageString = message.content();
		const QString& prefix = getGuildData()[channel.guildId()].prefix;
		QString fullCommand = messageString.mid(prefix.length());
		QString commandName = messageString.mid(prefix.length(), messageString.indexOf(QRegularExpression("\\s")) - prefix.length());

		bool isCommand = false;

		if (messageString.startsWith(prefix))
		{
			for (Module* module : modules)
			{
				for (const Command& command : module->getCommands())
				{
					if (command.name == commandName)
					{
						isCommand = true;
						
						::Permissions::contains(channel.guildId(), message.author().id(), command.requiredPermissions,
												[this, message, channel, command, fullCommand](bool result)
						{
							if (!result)
							{
								SEND_MESSAGE("You do not have permission to use this command!");
								return;
							}

							if (command.regex.match(fullCommand).hasMatch())
							{
								if (command.enabled)
								{

									command.callback(message, channel);

								}
								else
								{
									SEND_MESSAGE("This command has been disabled!");
								}
							}
							else
							{
								SEND_MESSAGE("Wrong usage of command!");
							}
						});

						break;
					}
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
	});
}
