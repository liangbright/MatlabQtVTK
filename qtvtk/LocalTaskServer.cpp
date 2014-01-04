#include <qtcpsocket.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <QFile>
#include <QDir>
#include <QDirIterator>

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
	qDebug() << "test example task read and write";

	TaskInformation Task;
	Task.Path = "M:/ExampleTasks/";
	Task.FolderName = "1234567890";
	QDir dir("M:/ExampleTasks");
	dir.mkdir(Task.FolderName);

	m_TaskHandler->WriteExampleTaskFile(Task);

	m_TaskHandler->ReadExampleTaskFile(Task);

	qDebug() << "example task test finished";

	qDebug() << "test QVtkFigure";

	auto Figure = new QVtkFigure(0);

	Figure->Show();

	auto points = vtkPoints::New();

	for (int i = 0; i < 10; ++i)
	{
		points->InsertPoint(i, double(100 + i), double(100 + i), double(100 + i));
	}

	auto Prop = Figure->PlotPoint(points);

	std::cout << "vtkProp Handle: " << Prop << std::endl;

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

	qDebug() << tr("The TaskServer is running on port %1.").arg(m_TcpServer->serverPort());

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

		auto TaskList = this->GetAllPendingTasks();
		for (int i = 0; i < TaskList.size(); ++i)
		{			
			auto Task = TaskList.at(i);

			auto IsSucess = m_TaskHandler->RunTask(Task);

			if (IsSucess == false)
				qDebug("Can not run the task");
			else
				qDebug("The task is sucessfully completed");

			this->AddProcessedTask(Task);
		}
		// close the connection
		socket->close();
		socket->deleteLater();
	}
}


bool LocalTaskServer::InitializeTaskFolders()
{
	QDir dir("M:");

	dir.mkdir("PendingTasks");

	dir.mkdir("ProcessedTasks");

	dir.mkdir("ExampleTasks");

	return true;
}


bool LocalTaskServer::HasPendingTasks()
{// check M:/PendingTasks if it is empty
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
	Task.Path = "M:/PendingTasks/";

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
// move *.json to "M:/CompletedTasks"
// delete Task folder
bool LocalTaskServer::AddProcessedTask(TaskInformation TaskInfo)
{

	QDir TaskDir(TaskInfo.Path + TaskInfo.FolderName);
	if (TaskDir.exists() == false)
	{
		qDebug() << "Strange: TaskDir does not exist";
		qDebug() << "Task.Path:" << TaskInfo.Path;
		qDebug() << "Task.FolderName:" << TaskInfo.FolderName;
		return false;
	}

	QString CompletedTaskPath = "M:/CompletedTasks/";

	QString tempFolder = CompletedTaskPath + "~" + TaskInfo.FolderName;

	QDir CompletedTaskDir(tempFolder);
	if (CompletedTaskDir.exists() == true)
	{
		CompletedTaskDir.removeRecursively();
	}

	auto Isthere = CompletedTaskDir.mkpath(tempFolder);

	if (Isthere == false)
	{
		qWarning() << "Fail to create temp CompletedTaskDir: " << tempFolder;
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
		auto sourceFile = TaskInfo.Path + TaskInfo.FolderName + "/" + sourceFileName;
		auto destinationFile = tempFolder + "/" + sourceFileName;
		auto tempresult = QFile::copy(sourceFile, destinationFile);

		if (tempresult == false)
		{
			result = false;
		}
	}

	CompletedTaskDir.rename(tempFolder, CompletedTaskPath + TaskInfo.FolderName);

	TaskDir.removeRecursively();

	return result;
}