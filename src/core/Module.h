#pragma once

#include <functional>
#include <QRegularExpression>
#include <Discord/Client.h>
#include <QString>

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
	Help,
	Echo,
	SetPrefix,
);

struct Command
{
	using Callback = std::function<void(const Discord::Message&, const Discord::Channel&)>;

	Commands id;
	bool enabled;
	unsigned int requiredPermissions;

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

	void registerCommand(Commands id, const QString& signature, unsigned int requiredPermissions, Command::Callback callback);

private:
	QString name;
	QList<Command> commands;
};
