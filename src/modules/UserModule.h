#pragma once

#include "core/Core.h"
#include "modules/Module.h"

class UserModule : public Module
{
public:
	UserModule();
	~UserModule();

	void setTimezone(const Discord::Message&, const Discord::Channel&);

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onStatus(QString& output, GuildId guildId, UserId userId) override;

private:
	QString getUserTimezoneString(UserId userId);
	
private:
	QMap<UserId, int /* seconds from UTC */> userData;
};
