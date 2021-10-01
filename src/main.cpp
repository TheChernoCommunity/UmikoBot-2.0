#include "core/InterruptHandler.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	InterruptHandler::init();
	
	qputenv("QT_QPA_PLATFORM_PLUGIN_PATH", ".");
	QApplication app { argc, argv };
	
	return app.exec();
}
