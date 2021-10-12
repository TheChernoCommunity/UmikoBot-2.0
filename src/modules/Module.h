#pragma once

#include <functional>
#include <QRegularExpression>
#include <Discord/Client.h>
#include <QString>
#include <QJsonObject>

// For use when registering commands
#define OPTIONAL(x) "(" x ")?"
#define SPACE "\\s+"

#define IDENTIFIER SPACE "\\S+"
#define USER SPACE "\\S+"

#define TEXT SPACE ".+"
#define INTEGER SPACE "[+-]?[0-9]+"
#define UNSIGNED_INTEGER SPACE "[+]?[0-9]+"
#define DECIMAL SPACE "[+-]?[0-9]+(\\.[0-9]+)?"
#define UNSIGNED_DECIMAL SPACE "[+]?[0-9]+(\\.[0-9]+)?"

// Intended to be used by modules
#define CALLBACK(name) [this](const Message& message, const Channel& channel){ name(message, channel); }
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
	Donate,
	Steal,
	Compensate,
	Richlist,
	Gamble,
	Bribe,
);

class Module;

struct Command
{
	using Callback = std::function<void(const Discord::Message&, const Discord::Channel&)>;

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

	virtual void onMessage(const Discord::Message& message, const Discord::Channel& channel) {};

	void save() const;
	void load();
	
	QList<Command>& getCommands() { return commands; }
	const QString& getName() const { return name; }

protected:
	Module(const QString& name);

	virtual void onSave(QJsonObject& mainObject) const {}
	virtual void onLoad(const QJsonObject& mainObject) {}
	
	void registerCommand(Commands id, const QString& signature, unsigned int requiredPermissions, Command::Callback callback);

private:
	QString name;
	QList<Command> commands;
};
