#include "core/Permissions.h"
#include "UmikoBot.h"

using namespace Discord;

void ::Permissions::contains(snowflake_t guildId, snowflake_t userId, unsigned int permissions, PermissionCallback callback)
{
	UmikoBot::get().getGuildMember(guildId, userId).then([guildId, userId, permissions, callback](const GuildMember& user)
	{
		if (UmikoBot::get().isOwner(guildId, userId))
		{
			return callback(true);
		}

		unsigned int totalPermissions = 0;
		for (const Role& role : UmikoBot::get().getRoles(guildId))
		{
			for (snowflake_t roleId : user.roles())
			{
				if (roleId == role.id())
				{
					totalPermissions |= role.permissions();
					break;
				}
			}
		}

		unsigned int x = 1;
		while (x <= permissions)
		{
			unsigned int current = permissions & x;
			if ((totalPermissions & current) != 0)
			{
				return callback(true);
			}

			x <<= 1;
		}

		return callback(false);
	});
}

void ::Permissions::matches(snowflake_t guildId, snowflake_t userId, unsigned int permissions, PermissionCallback callback)
{
	UmikoBot::get().getGuildMember(guildId, userId).then([guildId, userId, permissions, callback](const GuildMember& user)
	{
		if (UmikoBot::get().isOwner(guildId, userId))
		{
			return callback(true);
		}

		unsigned int totalPermissions = 0;
		for (const Role& role : UmikoBot::get().getRoles(guildId))
		{
			for (snowflake_t roleId : user.roles())
			{
				if (roleId == role.id())
				{
					totalPermissions |= role.permissions();
				}
			}
		}

		if ((totalPermissions & permissions) == permissions)
		{
			return callback(true);
		}

		return callback(false);
	});
}
