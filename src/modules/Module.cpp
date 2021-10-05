#include "Module.h"
#include "core/Permissions.h"

Module::Module(const QString& name)
	: name(name)
{
}

Module::~Module()
{
}

void Module::onMessage(const Discord::Message& message)
{
}

void Module::registerCommand(Commands id, const QString& signature, unsigned int requiredPermissions, Command::Callback callback)
{
	QRegularExpression regex { QString("^") + signature + "$" };
	QString name = signature.split(QRegularExpression("[\\s\\\\(]")).first();

	commands.push_back(Command { id, true, requiredPermissions, name, regex, callback });
}
