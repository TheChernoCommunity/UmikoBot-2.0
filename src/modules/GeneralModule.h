#pragma once

#include "modules/Module.h"

class GeneralModule : public Module
{
public:
	GeneralModule();

	void help(const Discord::Message&, const Discord::Channel&);
	void echo(const Discord::Message&, const Discord::Channel&);
	void status(const Discord::Message&, const Discord::Channel&);
	void setPrefix(const Discord::Message&, const Discord::Channel&);
	void enable(const Discord::Message&, const Discord::Channel&);
	void disable(const Discord::Message&, const Discord::Channel&);
	void setPrimaryChannel(const Discord::Message&, const Discord::Channel&);

	QString commandHelp(const QString& request, const QString& prefix);

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;
	
private:
	void enableDisableImpl(const Discord::Message&, bool enable);
};
