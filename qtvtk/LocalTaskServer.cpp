#include <qtcpsocket.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <QFile>

#include <memory>

#include "LocalTaskServer.h"

LocalTaskServer::LocalTaskServer()
{
	m_TaskHandler.reset(new TaskHandler());

	m_TcpServer.reset(new QTcpServer());

	m_TaskHandler->WriteExampleTaskFile("M:/CurrentTask/", "exampletask");

	m_TaskHandler->ReadExampleTaskFile("M:/CurrentTask/", "exampletask");
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
	auto socket = m_TcpServer->nextPendingConnection();

	qDebug("new connection: processing new task");	

	QString Prefix = "M:/PendingTask/";

	QString TaskHandle;
	//get the TaskHandle : e.g., M:/PendingTask/abcd12334345
	// the name abcd12334345 is the handle of the task 

	QString TaskFolderName = Prefix + TaskHandle;

	QString Path = TaskFolderName + '/';

	QString TaskFileName = "Task";

	auto IsSucess = m_TaskHandler->RunTask(Path, TaskFileName);
	if (IsSucess == false)
	{
		qDebug("Can not run the task");

		//copy *.json to  M:/FailedTask/TaskHandle";
		
		m_TaskHandler->RemoveFolder(TaskFolderName);
	}
	else
	{
		qDebug("The task is sucessfully completed");

		//copy *.json to  M:/CompletedTask/TaskHandle";

		m_TaskHandler->RemoveFolder(TaskFolderName);
	}

	// close the connection
	socket->close();
}



