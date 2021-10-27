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
		: UserWarning(warnedBy, QDateTime::currentDateTime(), message, true)
	{
	}

	UserWarning(UserId warnedBy, QDateTime when, QString message, bool isActive)
		: warnedBy(warnedBy), when(when), message(message), isActive(isActive)
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
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&) override;

private:
	unsigned int countWarningsForUser(UserId userId, bool activeWarningsOnly = true);
	void checkWarningsExpiry();
	
private:
	bool isModeratingInvitations = false;
	QMap<UserId, QList<UserWarning>> userWarnings;
};
