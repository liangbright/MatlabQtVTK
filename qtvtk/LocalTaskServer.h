#ifndef LocalTaskServer_h
#define LocalTaskServer_h

#include <qobject.h>
#include <qstring.h>
#include <qtcpServer.h>
#include <qhostaddress.h>

#include "QVtkFigure.h"
#include "TaskHandler.h"

class LocalTaskServer : public QObject 
{
	Q_OBJECT;

private:
	
    std::unique_ptr<QTcpServer> m_TcpServer;
	qint16 m_ServerPort = 12345;
	QHostAddress m_ServerAdress = QHostAddress::LocalHost;

	std::unique_ptr<TaskHandler> m_TaskHandler;

public:
	LocalTaskServer();
	~LocalTaskServer();
	
	void test();

	bool Startup();

	void Shutdown();

	bool ReadTaskHandle(unsigned long long*);

public slots:
	void HandleNewConnection();

private:
	bool InitializeTaskFolders();

	bool HasPendingTasks();

	std::vector<TaskInformation> GetAllPendingTasks();

	bool HandleFailedTask(TaskInformation);

	bool HandleCompletedTask(TaskInformation);
};

#endif