#ifndef QVtkFigure_h
#define QVtkFigure_h

#include <qobject.h>
#include <qstring.h>
#include <QTime>
#include <QMap>

#include <QVTKWidget.h>
#include <vtkPolyData.h>

//#include <ctime>
#include <unordered_map>

#include "QVtkFigureMainWindow.h"

class vtkProp;
class vtkVolumeProperty;
class vtkImageProperty;
class vtkPlane;

class DataInfomration
{
public:
	QString Name;

	double Range[2];

};

class PropInfomration
{
public:

	quint64 Handle = 0; //handle of the Prop

	QString Name;

	QString NameOnMenu; //default is ""

	vtkProp* Prop = nullptr;  

	QMenu* PropMenu = nullptr;

	vtkObject*  DataSource = nullptr;
};


class QVtkFigure : public QObject
{	
	Q_OBJECT;

private:

	quint64 m_Handle = 0; // FigureHandle

	QVtkFigureMainWindow* m_MainWindow = nullptr;

	QVTKWidget* m_QVtkWidget = nullptr;

	vtkRenderer* m_Renderer = nullptr;

	std::unordered_map<quint64, PropInfomration> m_PropRecord;

	quint64 m_PropHandleCounter = 0;

	QTime  m_time;

	quint64 m_AxesHandle = 0;

public:
	QVtkFigure();
	~QVtkFigure();

	void SetHandle(quint64 Handle)
	{
		m_Handle = Handle;
	}

	quint64 GetHandle() 
	{
		return m_Handle;
	}

	void SetTitle(QString Title);

	void CreateMenus();

	vtkRenderWindow* GetRenderWindow();

	vtkRenderer* GetRenderer();

	void AddProp(PropInfomration PropInfo);

	void RemoveProp(quint64 PropHandle);

	quint64 GeneratePropHandle();

	//---------------------------------------------------------------------
	quint64 ShowAxes();

	//---------------------------------------------------------------------

	quint64 PlotPoint(vtkPoints* PointData);

	vtkProp* CreatePointProp(vtkPoints* PointData);

	//---------------------------------------------------------------------
	quint64 ShowVolume(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty = nullptr);

	vtkProp* CreateVolumeProp(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty);

	vtkVolumeProperty* CreateDefaultVolumeProperty(const double DataRange[2]);

	QString GetDefaultRenderMethod();

	//----------------------------------------------------------------------

	quint64 QVtkFigure::ShowSliceOfVolume(quint64 VolumePropHandle, vtkPlane* SlicePlane, vtkImageProperty* ImageProperty = nullptr);

	vtkProp* QVtkFigure::CreateSliceOfVolumeProp(vtkImageData* VolumeData, vtkPlane* SlicePlane, vtkImageProperty* ImageProperty);

	vtkImageProperty* QVtkFigure::CreateDefaultImageProperty(const double DataRange[2]);
	//----------------------------------------------------------------------

	bool ResliceVolume(quint64 VolumePropHandle);

	//---------------------------------------------------------------------

	quint64 QVtkFigure::ShowPloyMesh(vtkPolyData* MeshData);

	vtkProp* CreatePloyMeshProp(vtkPolyData* MeshData);
	//----------------------------------------------------------------------

signals:
	void UserCloseFigure();

public slots:

    void ChangePropVisibility();

    void Show();
	
	void Close();

	void Save();

	void Refresh();
};

#endif