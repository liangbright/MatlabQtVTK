#include <qobject.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qboxlayout.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qfiledialog.h>

#include "QVtkFigure.h"
#include "QVtkFigureMainWindow.h"

QVtkFigureMainWindow::QVtkFigureMainWindow()
{	
}

QVtkFigureMainWindow::~QVtkFigureMainWindow()
{
	// Qt memory management cleans these up:
	//delete m_QVtkWidget;	
	//delete ???Action;
}

//------------------------Handle closeEvent-----------------------------------------
// Input:
//   event : user click "X"
// Output:
//   signal : UserCloseMainWindow
//------------------------------------------------------------------------------------------
void QVtkFigureMainWindow::closeEvent(QCloseEvent* event)
{
	emit(UserCloseMainWindow());
	//QMainWindow::closeEvent(event);
}

//------------------------Create menu of the mainwindow-----------------------------------------
// Input and Output: 
//   Figure   : connect signal
// Output:
//   m_FileMenu, m_SaveAction, m_PropListMenu
//------------------------------------------------------------------------------------------
void QVtkFigureMainWindow::CreateMenus(QVtkFigure *Figure)
{
	// file menu
	m_FileMenu = this->menuBar()->addMenu("&File");

	// save as image
	m_SaveAction = new QAction("&Save Window as ...", this);
	m_SaveAction->setShortcuts(QKeySequence::SaveAs);
	m_SaveAction->setStatusTip("Save Window as Image");

	m_FileMenu->addAction(m_SaveAction);
	connect(m_SaveAction, &QAction::triggered, Figure, &QVtkFigure::Save);

	// PropList menu
	m_PropListMenu = this->menuBar()->addMenu("PropList");
}

//------------------------Add menu of the prop-----------------------------------------
// Input and Output: 
// 1. PropInfo : assign pointer(PropMenu) to new prop menu 
// 2. Figure   : connect signal
//------------------------------------------------------------------------------------------
void QVtkFigureMainWindow::AddPropMenu(QVtkFigure* Figure, PropInfomration* PropInfo)
{	
	PropInfo->PropMenu = new QMenu(PropInfo->Name);

	m_PropListMenu->addMenu(PropInfo->PropMenu);

	auto Visibility = new QAction("&Visible", this);

	Visibility->setData(PropInfo->Handle);

	Visibility->setCheckable(true);

	Visibility->setChecked(true);

	PropInfo->PropMenu->addAction(Visibility);

	connect(Visibility, &QAction::triggered, Figure, &QVtkFigure::ChangePropVisibility);

}

//=============================Remove menu of the prop=======================================
// Input and Output: 
// 1. PropInfo : use pointer(PropMenu) to delete prop menu 
//------------------------------------------------------------------------------------------
void QVtkFigureMainWindow::RemovePropMenu(PropInfomration* PropInfo)
{
	if (PropInfo->PropMenu != nullptr)
   	    PropInfo->PropMenu->deleteLater();
}
