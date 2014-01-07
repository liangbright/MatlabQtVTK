#ifndef QVtkFigure_h
#define QVtkFigure_h

#include <qobject.h>
#include <qstring.h>
#include <QTime>

#include <vtkProp.h>
#include <vtkPolyData.h>
#include <QVTKWidget.h>

//#include <ctime>
#include <unordered_map>

#include "QVtkFigureMainWindow.h"

class vtkVolumeProperty;

class PropInfomration
{
public:

	quint64  Handle = 0; //handle of the Prop

	QString Name; //default is ""

	vtkProp* Prop = nullptr;  

	QMenu* PropMenu = nullptr;
};


class QVtkFigure : public QObject
{	
	Q_OBJECT;

private:

	quint64 m_Handle = 0;

	QVtkFigureMainWindow* m_MainWindow = nullptr;

	QVTKWidget* m_QVtkWidget = nullptr;

	vtkRenderer* m_Renderer = nullptr;

	std::unordered_map<quint64, PropInfomration> m_PropRecord;

	quint64 m_PropCounter;

	QTime  m_time;

public:
	QVtkFigure(quint64 Handle);
	~QVtkFigure();

	quint64 GetHandle() 
	{
		return m_Handle;
	}

	void CreateMenus();

	void SetTitle(QString Title);

	void AddProp(PropInfomration PropInfo);

	void RemoveProp(quint64 PropHandle);

	quint64 GeneratePropHandle();

	quint64 PlotPoint(vtkPoints* Point);
	vtkProp* CreatePointProp(vtkPoints* Point);

	quint64 ShowVolume(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty, QString RenderMethord);
	vtkProp* CreateVolumeProp(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty, QString RenderMethord);
	vtkVolumeProperty* GetDefaultVolumeProperty(double DataRange[2]);
	QString GetDefaultRenderMethod();

	quint64 QVtkFigure::ShowPloyMesh(vtkPolyData* MeshData, QString Color);
	vtkProp* CreatePloyMeshProp(vtkPolyData* MeshData, QString Color);

	vtkRenderWindow* GetRenderWindow();

	vtkRenderer* GetRenderer();

signals:
	void QVtkFigureClosed();

public slots:

    void ChangePropVisibility();

    void Show();
	
	void Close();

	void Save();

	void Refresh();
};

#endif