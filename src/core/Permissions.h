#pragma once

#include <functional>
#include <Discord/Client.h>

#include "core/Core.h"

namespace CommandPermission
{
	enum Permission
	{
		User = 0,
		Admin = Discord::Permissions::ADMINISTRATOR | Discord::Permissions::MANAGE_GUILD,
		Moderator = Admin | Discord::Permissions::MANAGE_MESSAGES | Discord::Permissions::KICK_MEMBERS | Discord::Permissions::BAN_MEMBERS,
	};
};

using PermissionCallback = std::function<void(bool)>;
void containsPermission(GuildId guildId, UserId userId, unsigned int permissions, PermissionCallback callback);
