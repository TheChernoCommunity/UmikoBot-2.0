#pragma once

#include "core/Core.h"
#include "modules/Module.h"

class LevelModule : public Module
{
public:
	LevelModule();
	~LevelModule();

protected:
	void onSave(QJsonObject& mainObject) const override;
	void onLoad(const QJsonObject& mainObject) override;

	void onMessage(const Discord::Message&, const Discord::Channel&);
};
