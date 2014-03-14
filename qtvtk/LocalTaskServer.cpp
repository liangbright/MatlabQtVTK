#include <qtcpsocket.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <QFile>
#include <QDir>
#include <QDirIterator>


#include <vtkImageData.h>

#include <memory>

#include "LocalTaskServer.h"


LocalTaskServer::LocalTaskServer()
{
	m_TaskHandler.reset(new TaskHandler());

	m_TcpServer.reset(new QTcpServer());

	this->InitializeTaskFolders();
}

void LocalTaskServer::test()
{
	QString Color("1.2 , 2.31, 0.11 ,9");

	qDebug() << "test QString split to number" << Color;

	auto ColorValue = Color.split(",");

	auto a1 = ColorValue.at(0).toDouble();
	if (a1 == 1.2)
		qDebug() << ColorValue.at(0).toDouble();

	auto a2 = ColorValue.at(1).toDouble();
	if (a2 == 2.31)
		qDebug() << ColorValue.at(1).toDouble();

	auto a3 = ColorValue.at(2).toDouble();
	if (a3 == 0.11)
		qDebug() << ColorValue.at(2).toDouble();

	auto a4 = ColorValue.at(3).toDouble();
	if (a4 == 9)
		qDebug() << ColorValue.at(3).toDouble();

	qDebug() << "test example task read and write";

	TaskInformation Task;
	Task.FolderPath = "M:/ExampleTasks/";
	Task.FolderName = "1234567890";
	QDir dir("M:/ExampleTasks");
	dir.mkdir(Task.FolderName);

	m_TaskHandler->WriteExampleTaskFile(Task);

	m_TaskHandler->ReadExampleTaskFile(Task);

	qDebug() << "example task test finished";

	qDebug() << "test QVtkFigure";

	auto Figure = new QVtkFigure();

	Figure->SetHandle(0);

	Figure->SetTitle("FigureHandle = 0");

	Figure->Show();

	//Figure->ShowAxes();

	auto points = vtkPoints::New();

	for (int i = 0; i < 10; ++i)
	{
		points->InsertPoint(i, double(i), double(i), double(i));
	}

	auto PointData = vtkPolyData::New();

	PointData->SetPoints(points);
	points->Delete();

	auto PointProp = Figure->PlotPoint(PointData);

	std::cout << "PointProp Handle: " << PointProp << std::endl;

	auto Image = vtkImageData::New();

	int ImageSize[3] = { 50, 100, 20 };

	Image->SetDimensions(ImageSize[0], ImageSize[1], ImageSize[2]);

	Image->AllocateScalars(VtkDataTypeEnum::VALUE_DOUBLE, 1);

	for (int z = 0; z < ImageSize[2]; ++z)
	{
		for (int y = 0; y < ImageSize[1]; ++y)
		{
			for (int x = 0; x < ImageSize[0]; ++x)
			{
				auto p = static_cast<double*>(Image->GetScalarPointer(x, y, z));
				if (z < 10)
				{
					p[0] = 1;
				}
				else
				{
					p[0] = 10;
				}				
			}
		}
	}

	double DataRange[2] = { 1, 10 };

	auto VolumeProperty = Figure->CreateDefaultVolumeProperty_Color(DataRange);

	auto ImageProp = Figure->ShowVolume(Image, VolumeProperty);

	std::cout << "ImageProp Handle: " << ImageProp << std::endl;

	// reset the camera center to object center,
	// then add axes
	Figure->ShowAxes();
}

LocalTaskServer::~LocalTaskServer()
{
}

bool LocalTaskServer::Startup()
{
	qDebug("start listening for connections...");

	if (!m_TcpServer->listen(m_ServerAdress, m_ServerPort))
	{
		QMessageBox::critical(NULL, tr("LocalTaskServer"),
			tr("Unable to start the LocalTaskServer: %1.").arg(m_TcpServer->errorString()));
		return false;
	}

	qDebug() << tr("The LocalTaskServer is listening on port %1").arg(m_TcpServer->serverPort());

	connect(m_TcpServer.get(), &QTcpServer::newConnection, this, &LocalTaskServer::HandleNewConnection);

	return true;
}


void LocalTaskServer::Shutdown()
{
	qDebug("shutting down LocalTaskServer...");
	m_TcpServer->close();

	m_TcpServer.reset();

	// close all figures, then Qapp will auto shutdown	
	m_TaskHandler.reset();
}


void LocalTaskServer::HandleNewConnection()
{	
	while (m_TcpServer->hasPendingConnections())
	{
		auto socket = m_TcpServer->nextPendingConnection();

		qDebug("new connection: processing new task");

		auto TaskInfoList = this->GetAllPendingTasks();
		for (int i = 0; i < TaskInfoList.size(); ++i)
		{			
			auto TaskInfo = TaskInfoList.at(i);

			auto IsSucess = m_TaskHandler->RunTask(TaskInfo);

			if (IsSucess == false)
				qDebug() << "Can not run the task : " << TaskInfo.FolderName;
			else
				qDebug("The task is sucessfully completed");

			this->AddProcessedTask(TaskInfo);
		}
		// close the connection
		socket->close();
		socket->deleteLater();
	}
}


bool LocalTaskServer::InitializeTaskFolders()
{
	QDir DiskDir("M:");
	if (DiskDir.exists() == false)
	{
        std::cout << '\a' << '\a' << '\a';
		qWarning("!!!!!!~~~~~~~~~~~~~~~~~ Disk M:/ does not exist ! ~~~~~~~~~~~~~~~~~~~~~~!!!!!!");
        std::cout << '\n';
        qWarning("Pleaase Create a Virtual Disk From Memory, and Name it as M");
        std::cout << '\n';
        qWarning("!!!!!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!!!!!!");
		return false;
	}

	QDir PendingTasksDir("M:/PendingTasks");
	if (PendingTasksDir.exists() == true)
	{	
		PendingTasksDir.removeRecursively();
	}
	DiskDir.mkdir("PendingTasks");
	
	QDir ProcessedTasksDir("M:/ProcessedTasks");
	if (ProcessedTasksDir.exists() == true)
	{
		ProcessedTasksDir.removeRecursively();
	}
	DiskDir.mkdir("ProcessedTasks");

	QDir ExampleTasksDir("M:/ExampleTasks");
	if (ExampleTasksDir.exists() == true)
	{
		ExampleTasksDir.removeRecursively();
	}
	DiskDir.mkdir("ExampleTasks");

	return true;
}


bool LocalTaskServer::HasPendingTasks()
{
    // check M:/PendingTasks if it is empty
	if (QDir("M:/PendingTasks").entryList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0)
	{
		return false;
	}

	return true;
}


std::vector<TaskInformation> LocalTaskServer::GetAllPendingTasks()
{
	std::vector<TaskInformation> TaskList;

	TaskInformation Task;
	Task.FolderPath = "M:/PendingTasks/";

	QDir dir("M:/PendingTasks");
	dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	auto list = dir.entryList();
	for (int i = 0; i < list.size(); ++i)
	{
		Task.FolderName = list.at(i);

		TaskList.push_back(Task);
	}

	return TaskList;
}

//================================ Move Processed Task files to another folder ===============
// move *.json from "M:/PendingTasks/xxx" to "M:/ProcessedTasks/xxx"
// delete Task folder "M:/PendingTasks/xxx"
bool LocalTaskServer::AddProcessedTask(TaskInformation TaskInfo)
{

	QDir TaskDir(TaskInfo.FolderPath + TaskInfo.FolderName);
	if (TaskDir.exists() == false)
	{
		qDebug() << "Strange: TaskDir does not exist";
		qDebug() << "Task.Path:" << TaskInfo.FolderPath;
		qDebug() << "Task.FolderName:" << TaskInfo.FolderName;
		return false;
	}

	QString ProcessedTaskPath = "M:/ProcessedTasks/";

	QString tempFolder = ProcessedTaskPath + "~temp~" + TaskInfo.FolderName;

	QDir ProcessedTaskDir(tempFolder);
	if (ProcessedTaskDir.exists() == true)
	{
		ProcessedTaskDir.removeRecursively();
	}

	auto Isthere = ProcessedTaskDir.mkpath(tempFolder);

	if (Isthere == false)
	{
		qWarning() << "Fail to create tempFoler in ProcessedTaskDir: " << tempFolder;
		// must delete completed task from "M:/pendingtasks/" 
		TaskDir.removeRecursively();
		return false;
	}

	TaskDir.setFilter(QDir::Files);
	QStringList name;
	name << "*.json";
	TaskDir.setNameFilters(name);
	auto list = TaskDir.entryList();

	bool result = true;
	for (int i = 0; i < list.size(); ++i)
	{
		auto sourceFileName = list.at(i);
		auto sourceFile = TaskInfo.FolderPath + TaskInfo.FolderName + "/" + sourceFileName;
		auto destinationFile = tempFolder + "/" + sourceFileName;
		auto tempresult = QFile::copy(sourceFile, destinationFile);

		if (tempresult == false)
		{
			result = false;
		}
	}

	ProcessedTaskDir.rename(tempFolder, ProcessedTaskPath + TaskInfo.FolderName);

	TaskDir.removeRecursively();

	return result;
}