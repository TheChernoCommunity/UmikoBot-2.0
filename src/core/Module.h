#pragma once

#include <functional>
#include <QRegularExpression>
#include <Discord/Client.h>

enum class Commands
{
	// Global Module
	Help,
};

struct Command
{
	using Callback = std::function<void(const Discord::Message&, const Discord::Channel&)>;

	Commands id;
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
	const QString& getName() const { return name; }

protected:
	Module(const QString& name);

	void registerCommand(Commands id, const QString& signature, Command::Callback callback);

private:
	QString name;
	QList<Command> commands;
};
