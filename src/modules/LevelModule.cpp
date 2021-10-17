#include "LevelModule.h"
#include "UmikoBot.h"
#include "core/Permissions.h"

using namespace Discord;

LevelModule::LevelModule()
	: Module("Level")
{
	namespace CP = CommandPermission;
}

LevelModule::~LevelModule()
{
}

void LevelModule::onSave(QJsonObject& mainObject) const
{
}

void LevelModule::onLoad(const QJsonObject& mainObject)
{
}

void LevelModule::onMessage(const Message& message, const Channel& channel)
{
}
