#pragma once

#include "core/Core.h"
#include "modules/Module.h"

#include <QDateTime>

struct UserWarning
{
	UserId warnedBy;
	QDateTime when;
	QString message;
	bool isActive;

	UserWarning(UserId warnedBy, QString message)
		: UserWarning(warnedBy, QDateTime::currentDateTime(), message)
	{
	}

	UserWarning(UserId warnedBy, QDateTime when, QString message)
		: warnedBy(warnedBy), when(when), message(message), isActive(true)
	{
	}
};

class ModerationModule : public Module
{
public:
	ModerationModule();

	void moderateInvitations(const Discord::Message&, const Discord::Channel&);
	void warn(const Discord::Message&, const Discord::Channel&);
	void warnings(const Discord::Message&, const Discord::Channel&);

protected:
	void onMessage(const Discord::Message&, const Discord::Channel&) override;

private:
	unsigned int countWarningsForUser(UserId userId, bool activeWarningsOnly = true);
	
private:
	bool isModeratingInvitations = false;
	QMap<UserId, QList<UserWarning>> userWarnings;
};
