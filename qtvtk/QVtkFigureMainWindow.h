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

	void closeEvent(QCloseEvent* event);

	void CreateMenus(QVtkFigure* Figure);

	void AddPropMenu(QVtkFigure* Figure, PropInfomration* PropInfo);

	void RemovePropMenu(PropInfomration* PropInfo);

signals:
	void UserCloseMainWindow();
};

#endif