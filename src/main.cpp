#include <QtWidgets/QApplication>

#include "UmikoBot.h"
#include "core/InterruptHandler.h"

int main(int argc, char* argv[])
{
	InterruptHandler::init();
	
	qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", ".");
	QApplication app { argc, argv };

	// Retrieves the authorisation token
	QStringList arguments = app.arguments();
	if (arguments.count() < 2)
	{
		printf("No token was provided!\n");
		return -1;
	}
	else if (arguments.count() > 2)
	{
		printf("Please only provide a single argument (the token).\n");
		return -1;
	}
	
	// Log in
	Discord::Token token;
	token.generate(arguments[1], Discord::Token::Type::BOT);
	UmikoBot::get().login(token);
	
	return app.exec();
}
