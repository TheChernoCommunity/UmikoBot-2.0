#pragma once

#include <functional>
#include <QRegularExpression>
#include <Discord/Client.h>

struct Command
{
	using Callback = std::function<void(const Discord::Message&, const Discord::Channel&)>;

	unsigned int id;
	bool enabled;
	// TODO(fkp): Permission

	QString name;
	QRegularExpression regex;
	Callback callback;
};

class Module
{
public:
	virtual ~Module();

	void onMessage(const Discord::Message& message);
	QList<Command>& getCommands() { return commands; }

protected:
	Module(const QString& name);

	void registerCommand(unsigned int id, const QString& signature, Command::Callback callback);

private:
	QString name;
	QList<Command> commands;
};
