#pragma once

#include <functional>
#include <Discord/Client.h>

#include "core/Core.h"

namespace CommandPermission
{
	enum Permission
	{
		Admin = Discord::Permissions::ADMINISTRATOR | Discord::Permissions::MANAGE_GUILD,
		Moderator = Admin | Discord::Permissions::MANAGE_MESSAGES | Discord::Permissions::KICK_MEMBERS | Discord::Permissions::BAN_MEMBERS,
		User = 0xffffffff,
	};
};

using PermissionCallback = std::function<void(bool)>;

class Permissions
{
public:
	static void contains(GuildId guildId, UserId userId, unsigned int permissions, PermissionCallback callback);
	static void matches(GuildId guildId, UserId userId, unsigned int permissions, PermissionCallback callback);
};
