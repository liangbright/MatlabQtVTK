#ifndef QVtkFigure_h
#define QVtkFigure_h

#include <qobject.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qstring.h>

#include <vtkProp.h>
#include <vtkPolyData.h>

#include <QVTKWidget.h>

#include <ctime>
#include <unordered_map>

class PropInfomration
{
public:

	quint64  Handle = 0; //handle of the Prop

	vtkDataObject* DataSource;

	vtkProp* Prop = nullptr;

	QString Name = "";

	QAction* MenuAction = nullptr;
};

class QVtkFigure : public QObject
{	
	Q_OBJECT;

private:
	QMainWindow *m_MainWindow = nullptr;

	QVTKWidget *m_QVtkWidget = nullptr;

	vtkRenderer *m_Renderer = nullptr;

	QMenu *m_FileMenu = nullptr;
	QMenu *m_PropMenu = nullptr;

	QAction *m_CloseAction = nullptr;
	QAction *m_SaveAction = nullptr;

	std::unordered_map<quint64, PropInfomration> m_PropRecord;

public:
	QVtkFigure();
	~QVtkFigure();

	void CreateMenus();

	void SetTitle(QString Title);

	void AddProp(PropInfomration PropInfo);

	void RemoveProp(unsigned long long);

	quint64 GeneratePropHandle();

	vtkProp* CreatePointProp(vtkPoints*);

	quint64 PlotPoint(vtkPoints*);

	vtkProp* CreateImageProp(vtkImageData*);

	quint64 ShowImage(vtkImageData*);

	vtkRenderWindow* GetRenderWindow();

	vtkRenderer* GetRenderer();

signals:
	void QVtkFigureDeleted();

public slots:

	void Close();

	void Show();

	void Save();

	void Refresh();
};

#endif