#include "ModerationModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

ModerationModule::ModerationModule()
	: Module("Moderation")
{
	namespace CP = CommandPermission;
	registerCommand(Commands::ModerateInvitations, "moderate-invitations (on|off)", CP::Moderator, CALLBACK(moderateInvitations));
}

void ModerationModule::onMessage(const Message& message, const Channel& channel)
{
	if (isModeratingInvitations)
	{
		if (message.content().contains("discord.gg/", Qt::CaseInsensitive))
		{
			::Permissions::contains(channel.guildId(), message.author().id(), CommandPermission::Moderator, [this, message](bool result)
			{
				if (result)
				{
					// Moderators are excluded from this
					return;
				}

				UmikoBot::get().deleteMessage(message.channelId(), message.id());
				UmikoBot::get().createDm(message.author().id()).then([this, message](const Channel& channel)
				{
					UmikoBot::get().createMessage(channel.id(), "Invitation links to discord servers are not allowed. Please take it to DMs!\n"
												  "Here is the message which you posted:");
					UmikoBot::get().createMessage(channel.id(), message.content());
				});
			});
		}
	}
}

void ModerationModule::moderateInvitations(const Message& message, const Channel& channel)
{
	(void) channel;
	QStringList args = message.content().split(QRegularExpression(SPACE));
	if (args[1] == "on")
	{
		isModeratingInvitations = true;
	}
	else if (args[1] == "off")
	{
		isModeratingInvitations = false;
	}

	SEND_MESSAGE(QString("Moderating invitations has been turned %1!").arg(args[1]));
}
