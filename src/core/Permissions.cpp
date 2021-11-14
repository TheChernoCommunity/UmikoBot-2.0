#include "core/Permissions.h"
#include "UmikoBot.h"

using namespace Discord;

void containsPermission(GuildId guildId, UserId userId, unsigned int permissions, PermissionCallback callback)
{
	UmikoBot::get().getGuildMember(guildId, userId).then([guildId, userId, permissions, callback](const GuildMember& user)
	{
		if (UmikoBot::get().isOwner(guildId, userId))
		{
			return callback(true);
		}

		if (permissions == 0)
		{
			return callback(true);
		}

		for (RoleId roleId : user.roles())
		{
			for (const Role& role : UmikoBot::get().getRoles(guildId))
			{
				if (role.id() == roleId)
				{
					if ((permissions & role.permissions()) != 0)
					{
						return callback(true);
					}

					break;
				}
			}
		}

		return callback(false);
	});
}
