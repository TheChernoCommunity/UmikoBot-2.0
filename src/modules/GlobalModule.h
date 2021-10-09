#pragma once

#include "modules/Module.h"

class GlobalModule : public Module
{
public:
	GlobalModule();
	~GlobalModule();

	static void help(Module*, const Discord::Message&, const Discord::Channel&);
	static void echo(Module*, const Discord::Message&, const Discord::Channel&);
	static void setPrefix(Module*, const Discord::Message&, const Discord::Channel&);
	static void enable(Module*, const Discord::Message&, const Discord::Channel&);
	static void disable(Module*, const Discord::Message&, const Discord::Channel&);

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;
	
private:
	static void enableDisableImpl(Module*, const Discord::Message&, const Discord::Channel&, bool enable);
};
