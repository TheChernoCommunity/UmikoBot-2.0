#include "Module.h"
#include "core/Permissions.h"

#include <QFile>

Module::Module(const QString& name)
	: name(name)
{
}

Module::~Module()
{
}

void Module::save() const
{
	QString path = "configs/" + name + ".json";
	QFile file { path };
	if (file.open(QFile::ReadWrite | QFile::Truncate))
	{
		QJsonDocument document;
		onSave(document);
		
		file.write(document.toJson(QJsonDocument::Indented));
		file.close();
	}
	else
	{
		printf("Failed to open '%s' for saving.\n", path.toStdString().c_str());
	}
}

void Module::load()
{
	QString path = "configs/" + name + ".json";
	QFile file { path };
	if (file.open(QFile::ReadOnly))
	{
		onLoad(QJsonDocument::fromJson(file.readAll()));
		file.close();
	}
	else
	{
		printf("Failed to open '%s' for loading.\n", path.toStdString().c_str());
	}
}

void Module::registerCommand(Commands id, const QString& signature, unsigned int requiredPermissions, Command::Callback callback)
{
	QRegularExpression regex { QString("^") + signature + "$" };
	QString name = signature.split(QRegularExpression("[\\s\\\\(]")).first();

	commands.push_back(Command { id, true, requiredPermissions, name, regex, callback });
}
