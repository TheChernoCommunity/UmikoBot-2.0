#include "Module.h"

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

void Module::registerCommand(unsigned int id, const QString& signature, Command::Callback callback)
{
	QRegularExpression regex { QString("!") + signature }; // TODO(fkp): Variable prefix
	QString name = signature.split(' ').first();
	
	commands.push_back(Command { id, true, name, regex, callback });
}
