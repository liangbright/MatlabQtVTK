#include <qapplication.h>
#include <qdebug.h>

#include <vtkSmartPointer.h>

#include <memory>

#include "QVtkFigure.h"
#include "LocalTaskServer.h"

int main(int argc, char** argv)
{
	qDebug("start");

	QApplication app(argc, argv);
	
	auto Figure = std::unique_ptr<QVtkFigure>(new QVtkFigure);

	Figure->Show();

	auto points = vtkSmartPointer<vtkPoints>::New();

	for (int i = 0; i < 10; ++i)
	{
		points->InsertPoint(i, double(100 + i), double(100 + i), double(100 + i));
	}

	auto Prop = Figure->PlotPoint(points);

	std::cout << "vtkProp Handle: " << Prop << std::endl;
	//----------------------------------------------------------------//

	auto Server = std::unique_ptr<LocalTaskServer>(new LocalTaskServer);

	Server->Startup();

	return app.exec();
}