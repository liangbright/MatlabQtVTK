#ifndef QVtkFigureMainWindow_h
#define QVtkFigureMainWindow_h

#include <qmainwindow.h>
#include <qmenu.h>
#include <qstring.h>

class QVtkFigure;
class PropInfomration;

class QVtkFigureMainWindow : public QMainWindow
{	
	Q_OBJECT;

private:

	QMenu *m_FileMenu = nullptr;

	QMenu *m_PropListMenu = nullptr;

	QAction *m_SaveAction = nullptr;

public:
	QVtkFigureMainWindow();
	~QVtkFigureMainWindow();

	void closeEvent(QCloseEvent*);

	void CreateMenus(QVtkFigure*);

	void AddPropMenu(QVtkFigure*, PropInfomration*);

	void RemovePropMenu(PropInfomration*);

signals:
	void UserCloseMainWindow();
};

#endif