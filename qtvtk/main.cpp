#include <qapplication.h>
#include <qdebug.h>

#include <memory>

#include "QVtkFigure.h"
#include "LocalTaskServer.h"

int main(int argc, char** argv)
{
	qDebug("start");

	QApplication app(argc, argv);

	app.setQuitOnLastWindowClosed(false);
	
	auto Server = std::unique_ptr<LocalTaskServer>(new LocalTaskServer);

	Server->test();

	Server->Startup();
	
	return app.exec();
}