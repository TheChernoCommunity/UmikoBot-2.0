#pragma once

#include <functional>
#include <QRegularExpression>
#include <Discord/Client.h>
#include <QString>
#include <QJsonDocument>

// For use when registering commands
#define OPTIONAL(x) "(" x ")?"
#define SPACE "\\s+"
#define IDENTIFIER SPACE "\\S+"
#define TEXT SPACE ".+"
#define USER SPACE "\\S+"

// Intended to be used by modules
#define SEND_MESSAGE(msg) UmikoBot::get().createMessage(message.channelId(), msg);

#define CREATE_COMMANDS(...)											\
	enum class Commands													\
	{																	\
		__VA_ARGS__														\
		Count															\
	};																	\
																		\
	struct CommandInfo													\
	{																	\
		inline static QString enumFullString = #__VA_ARGS__;			\
																		\
		/* These rely on the count enum value at the end */				\
		inline static QString commandStrings[(unsigned int) Commands::Count] = {}; \
		inline static QString briefDescription[(unsigned int) Commands::Count] = {}; \
		inline static QString usage[(unsigned int) Commands::Count] = {}; \
		inline static QString additionalInfo[(unsigned int) Commands::Count] = {}; \
		inline static bool adminRequired[(unsigned int) Commands::Count] = {}; \
	};

CREATE_COMMANDS(
	// Global Module
	Help,
	Echo,
	SetPrefix,
	Enable,
	Disable,

	// Currency Module
	Wallet,
	Daily,
);

class Module;

struct Command
{
	using Callback = std::function<void(Module*, const Discord::Message&, const Discord::Channel&)>;

	Commands id;
	bool enabled; 	// TODO(fkp): Enabled state per guild
	unsigned int requiredPermissions;

	QString name;
	QRegularExpression regex;
	Callback callback;
};

class Module
{
public:
	virtual ~Module();

	virtual void onMessage(const Discord::Message& message) {};

	void save() const;
	void load();
	
	QList<Command>& getCommands() { return commands; }
	const QString& getName() const { return name; }

protected:
	Module(const QString& name);

	virtual void onSave(QJsonDocument& doc) const {}
	virtual void onLoad(const QJsonDocument& doc) {}
	
	void registerCommand(Commands id, const QString& signature, unsigned int requiredPermissions, Command::Callback callback);

private:
	QString name;
	QList<Command> commands;
};
