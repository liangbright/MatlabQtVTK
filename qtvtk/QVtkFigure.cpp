#include <qobject.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qaction.h>
#include <qboxlayout.h>
#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdesktopservices.h>
#include <qDebug>

#include <QVTKWidget.h> 
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkConeSource.h>
#include <vtkImageProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkVolumeMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkVolumeTextureMapper3D.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkImageResliceMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkImageSlice.h>
#include <vtkCamera.h>
#include <vtkRenderer.h> 
#include <vtkRenderWindow.h>
//#include <vtkDelaunay3D.h>
//#include <vtkGeometryFilter.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>

#include "QVtkFigureMainWindow.h"
#include "QVtkFigure.h"

QVtkFigure::QVtkFigure()
{
	m_MainWindow = new QVtkFigureMainWindow();
	
	m_QVtkWidget = new QVTKWidget();

	m_MainWindow->setCentralWidget(m_QVtkWidget);

	m_MainWindow->resize(512, 512);

	m_QVtkWidget->resize(m_MainWindow->size());

	m_QVtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	//vtk-----------------------------------------------------
	m_Renderer = vtkRenderer::New();

	m_QVtkWidget->GetRenderWindow()->AddRenderer(m_Renderer);

	m_Renderer->Delete();
	//-----------------------------------------------------//

	m_MainWindow->CreateMenus(this);

	connect(m_MainWindow, &QVtkFigureMainWindow::UserCloseMainWindow, this, &QVtkFigure::UserCloseFigure);

	m_time.start();
}

QVtkFigure::~QVtkFigure()
{
	m_MainWindow->deleteLater();

	// Qt memory management :
	// m_MainWindow delelets m_QVtkWidget

	// Vtk memory management :
	// m_QVtkWidget deletes m_Renderer
	// m_Renderer deletes the props
	// the props delete the others
}


void QVtkFigure::Refresh()
{
	//this->_renderer->Render();
	this->m_QVtkWidget->update();
	//this->vtkWidget->GetRenderWindow()->Render();
}

void QVtkFigure::Show()
{
	this->m_MainWindow->show();
}

void QVtkFigure::Save()
{
	/*
	qDebug("saveing window.");

	QString docDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).append("/");

	const QString caption = "Save window as image";
	const QString dir = docDir.append(QString("SceneWindow_%1.png").arg(SceneRegistry::instance()->handleFor(this)));

	const QString ImageFormat = tr("PNG Image (*.png) ;; JPEG Image (*.jpg)");
	QString SelectedImageFormat = "PNG Image (*.png)";

	//Options option = 0;
	QString saveFile = QFileDialog::getSaveFileName(centralWidget, caption, dir, filter, &selectedFilter);
	qDebug() << "saving to:" << saveFile << " as: " << selectedFilter;

	// no file was selected, user clicked "cancel": return without doing anything
	if (saveFile.length() == 0) {
	Debug("cancel @ QVtkFigure::Save()");
	return;
	}

	vtkSmartPointer<vtkWindowToImageFilter> imageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
	imageFilter->SetInput(this->vtkWidget->GetRenderWindow());

	vtkSmartPointer<vtkImageWriter> imageWriter;
	if (selectedFilter == "PNG Image (*.png)")
	{
	imageWriter = vtkSmartPointer<vtkPNGWriter>::New();
	}
	else if (selectedFilter == "JPEG Image (*.jpg)")
	{
	imageWriter = vtkSmartPointer<vtkJPEGWriter>::New();
	}
	else
	{
	//Default to PNG writer
	imageWriter = vtkSmartPointer<vtkPNGWriter>::New();
	}

	imageWriter->SetInputData(imageFilter->GetOutput());

	imageWriter->SetFileName(saveFile.toStdString().c_str());
	imageWriter->Write();
	*/
}

void QVtkFigure::Close()
{
	// inform Taskhandler.m_FigureRecord to delete this figure
	emit(UserCloseFigure());
}


void QVtkFigure::SetTitle(QString Title)
{
	m_MainWindow->setWindowTitle(Title);
}


vtkRenderWindow* QVtkFigure::GetRenderWindow()
{
	return m_QVtkWidget->GetRenderWindow();
}


vtkRenderer* QVtkFigure::GetRenderer()
{
	return m_Renderer;
}


quint64 QVtkFigure::GeneratePropHandle()
{
	m_PropHandleCounter = m_PropHandleCounter + 1;

	return m_PropHandleCounter;

	/*
	auto StartTime = m_time.elapsed();
	auto EndTime = StartTime;

	while (true)
	{
		EndTime = m_time.elapsed();

		if (EndTime - StartTime >= 1)
		{
			break;
		}
	}

	return (quint64)EndTime;
	*/

	/*  only 1 handler per sec 
	auto StartTime = std::time(nullptr); // time_t to double -> overflow ?

	auto EndTime = StartTime;

	while (true)
	{
		EndTime = std::time(nullptr);

		if (EndTime - StartTime >= 1)
		{
			break;
		}
	}

	return (quint64)EndTime;
	*/
}


void QVtkFigure::AddProp(PropInfomration PropInfo)
{
	bool IsFirst = false;
	if (m_PropRecord.empty())
	{
		IsFirst = true;
	}

	auto it = m_PropRecord.find(PropInfo.Handle);
	if (it == m_PropRecord.end())
	{
		QString tempText = "<" + PropInfo.Name+ ">";

		this->m_Renderer->AddViewProp(PropInfo.Prop);

		PropInfo.Prop->Delete();

		m_MainWindow->AddPropMenu(this, &PropInfo);

		m_PropRecord[PropInfo.Handle] = PropInfo;

		// when adding first component: automatically reset view
		if (IsFirst)
		{
			m_Renderer->ResetCamera();
			m_QVtkWidget->GetRenderWindow()->Render();
		}

		this->Refresh();
	}
}


void QVtkFigure::RemoveProp(quint64 PropHandle)
{
	auto it = m_PropRecord.find(PropHandle);
	if (it != m_PropRecord.end())
	{
		auto PropInfo = it->second;

		if (PropInfo.Name == "Axes")
		{
			m_AxesHandle = 0;
		}

		m_MainWindow->RemovePropMenu(&PropInfo);

		m_Renderer->RemoveViewProp(PropInfo.Prop);

		m_PropRecord.erase(it);

		this->Refresh();
	}
}


void QVtkFigure::ChangePropVisibility()
{
	auto Visibility = dynamic_cast<QAction*>(QObject::sender());
	if (Visibility == nullptr)
	{
		return;
	}

	quint64 PropHandle = Visibility->data().toULongLong();

	auto it = m_PropRecord.find(PropHandle);
	if (it != m_PropRecord.end())
	{
		auto PropInfo = it->second;

		auto status = PropInfo.Prop->GetVisibility();
		if (status == 1)
			PropInfo.Prop->SetVisibility(0);
		else
			PropInfo.Prop->SetVisibility(1);

		//m_Renderer->ResetCamera();
        //m_QVtkWidget->GetRenderWindow()->Render();
		
		this->Refresh();
	}

}


quint64 QVtkFigure::ShowAxes()
{
	// only one set of axes
	if (m_AxesHandle > 0)
	{
		return m_AxesHandle;
	}

	auto axes = vtkAxesActor::New();

	axes->SetTotalLength(100,100,100);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Axes";

	PropInfo.NameOnMenu = "Axes(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = axes;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


//===================================== Plot Point =====================================
// Input:
//   Points: 
// Output
//   PropInfo.Handle : 0 if something goes wrong; >0 is good
//---------------------------------------------------------------------------------------
quint64 QVtkFigure::PlotPoint(vtkPoints* PointData)
{
	auto Prop = this->CreatePointProp(PointData);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Point";

	PropInfo.NameOnMenu = "Points(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	PropInfo.DataSource = PointData;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreatePointProp(vtkPoints *PointData)
{
	//qDebug() << "vtk points: " << PointData;

	//qDebug() << "Point refrence counter is " << PointData->GetReferenceCount(); //1

	auto PolyData = vtkPolyData::New(); // reference counter of PolyData is 1
	//qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//1
		
	PolyData->SetPoints(PointData);// increase the reference counter of Point
	//qDebug() << "Point refrence counter is " << PointData->GetReferenceCount();//2

	auto Glyph = vtkSmartPointer<vtkGlyph3D>::New();

	Glyph->SetInputData(0, PolyData);//increase the reference counter of PolyData

	//qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//3
	
	PolyData->Delete();//decrease the reference counter of PolyData

	//qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//2

	// draw spheres at the points
	auto Sphere = vtkSmartPointer<vtkSphereSource>::New();

	Glyph->SetInputConnection(1, Sphere->GetOutputPort());

	Glyph->ScalingOff();
	Glyph->OrientOn();
	Glyph->ClampingOff();
	Glyph->SetVectorModeToUseVector();
	Glyph->SetIndexModeToOff();

	// set up mapper that shows shperes at points
	auto GlyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	GlyphMapper->SetInputConnection(Glyph->GetOutputPort());

	auto GlyphActor = vtkActor::New(); //refrence counter of GlyphActor is 1
	GlyphActor->SetMapper(GlyphMapper);

	//decrease the reference counter of Point
	PointData->Delete();
	//qDebug() << "Point refrence counter is " << PointData->GetReferenceCount();//1

	//upcast to vktProp*, dynamic_cast is not necessary
	return GlyphActor;  

	/* wrong 
	auto GlyphActor = vtkSmartPointer<vtkActor>::New();
	GlyphActor->SetMapper(GlyphsMapper);
	vtkProp* Prop = GlyphActor;
	return GlyphActor;
	// crash here because GlyphActor is deleted
	*/
}

//======================================= Show Volume Image ==============================================================
quint64 QVtkFigure::ShowVolume(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty)
{
	auto Prop = this->CreateVolumeProp(VolumeData, VolumeProperty);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Volume";

	PropInfo.NameOnMenu = "Volume(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	PropInfo.DataSource = VolumeData;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}

vtkProp* QVtkFigure::CreateVolumeProp(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty = nullptr)
{

	// fast but limited renderer: samples volume down
	// raycasting may be nicer but is limited on data types
	//vtkSmartVolumeMapper *smartVolumeMapper = vtkSmartVolumeMapper::New();

	// do not use vtkSmartPointer (then .take()) here
	vtkVolumeMapper* VolumeMapper = nullptr;

	vtkGPUVolumeRayCastMapper *GpuRcMapper = vtkGPUVolumeRayCastMapper::New();

	if (GpuRcMapper->IsRenderSupported(this->GetRenderWindow(), VolumeProperty)) 
	{
		qDebug("GPU OpenGL RayCasting enabled!");
		VolumeMapper= GpuRcMapper;
	}
	// no fast GPU raycasting available.
	else 
	{
		GpuRcMapper->Delete();

		qDebug("SLOW software ray casting.");
		//vtkVolumeRayCastCompositeFunction *compositeFunction = vtkVolumeRayCastCompositeFunction::New();
		
		auto *RcMapper = vtkFixedPointVolumeRayCastMapper::New();
		
		RcMapper->SetBlendModeToComposite();

		//raycastVolumeMapper->SetVolumeRayCastFunction(compositeFunction);
		//compositeFunction->Delete();
		//raycastVolumeMapper->AutoAdjustSampleDistancesOn();
			
		//Debug() << "sample distance" << raycastVolumeMapper->GetSampleDistance();

		VolumeMapper= RcMapper;
	}

	VolumeMapper->SetInputData(VolumeData);

	auto VolumeProp = vtkVolume::New();

	VolumeProp->SetMapper(VolumeMapper);

	if (VolumeProperty != nullptr)
	{
		VolumeProp->SetProperty(VolumeProperty);
		VolumeProperty->Delete();
	}
	else
	{
		double DataRange[2] = { 0, 0 };
		VolumeData->GetScalarRange(DataRange);

		auto tempVolumeProperty = this->CreateDefaultVolumeProperty(DataRange);

		VolumeProp->SetProperty(tempVolumeProperty);
		tempVolumeProperty->Delete();
	}
	
	//VolumeProp->Modified();

	VolumeMapper->Delete();
	

	// VolumeData is created by NEW(), and then is owned by VolumeMapper.
	VolumeData->Delete();

	return VolumeProp;
	
}

QString QVtkFigure::GetDefaultRenderMethod()
{
	return QString("RayCast");
}

vtkVolumeProperty* QVtkFigure::CreateDefaultVolumeProperty(const double DataRange[2])
{
	auto VolumeProperty = vtkVolumeProperty::New();

	if (DataRange[1] < DataRange[0])
	{
		qWarning() << "DataRange[1] < DataRange[0] @ CreateDefaultVolumeProperty";

		return VolumeProperty; // keep it simple, nullptr will lead to crash
	}

	auto ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

	auto OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	
	qDebug("default color LUT for range: %.2f - %.2f", DataRange[0], DataRange[1]);
	//colorTransferFunction->HSVWrapOff();
	//colorTransferFunction->SetColorSpaceToHSV();

	ColorTransferFunction->SetColorSpaceToRGB();

	//AddHSVSegment (double x1, double h1, double s1, double v1, double x2, double h2, double s2, double v2)
	//colorTransferFunction->AddHSVPoint(range[0], 0.6667, 1, 1);
	//colorTransferFunction->AddHSVPoint(range[1], 0, 1, 1);

	double dataMin = DataRange[0];
	double dataMax = DataRange[1];
	double dataDiff = (dataMax - dataMin);

	// matlab like jet color table
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.000), 0.0, 0.0, 0.5);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.125), 0.0, 0.0, 1.0);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.375), 0.0, 1.0, 1.0);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.625), 1.0, 1.0, 0.0);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.875), 1.0, 0.0, 0.0);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*1.000), 0.5, 0.0, 0.0);
	
	// create default opacity lut, 
	double opacity = 0;
	double step = (DataRange[1] - DataRange[0])/100;
	for (double level = DataRange[0]; level < DataRange[1]; level += step)
	{
		opacity = 0.01 + 0.99*(level - DataRange[0]) / (DataRange[1] + 0.01);
		opacity = opacity*0.5;
		OpacityTransferFunction->AddPoint(level, opacity);
	}
	//constant opacities over whole data range
	//double opacity = 0.05;
	//OpacityTransferFunction->AddPoint(DataRange[0], opacity);
	//OpacityTransferFunction->AddPoint(DataRange[1], opacity);

	VolumeProperty->SetColor(ColorTransferFunction);
	
	VolumeProperty->SetScalarOpacity(OpacityTransferFunction);

	return VolumeProperty;
}


quint64 QVtkFigure::ShowSliceOfVolume(quint64 VolumePropHandle, vtkPlane* SlicePlane, vtkImageProperty* ImageProperty)
{
	auto it = m_PropRecord.find(VolumePropHandle);
	if (it == m_PropRecord.end())
	{
		qWarning() << "PropHandle is invalid @ ShowSliceOfVolume";
		return 0;
	}

	auto VolumePropInfo = it->second;
	if (VolumePropInfo.Name != "Volume")
	{
		qWarning() << "PropHandle does not match Prop @ ShowSliceOfVolume";
		return 0;
	}

	auto VolumeData = static_cast<vtkImageData*>(VolumePropInfo.DataSource);
	
    //--------------------------------------------------------------------------

	auto Prop = this->CreateSliceOfVolumeProp(VolumeData, SlicePlane, ImageProperty);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "SliceOfVolume";

	PropInfo.NameOnMenu = "SliceOfVolume(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	PropInfo.DataSource = VolumeData;

	this->AddProp(PropInfo);

	return PropInfo.Handle;

}

vtkProp* QVtkFigure::CreateSliceOfVolumeProp(vtkImageData* VolumeData, vtkPlane* SlicePlane, vtkImageProperty* ImageProperty)
{
	// the origin in SlicePlane is relative to VolumeData
	// change it to the world coordinate system

	double VolumeOrigin[3] = { 0, 0, 0 };		
	VolumeData->GetOrigin(VolumeOrigin);
	
	double PlaneOrigin[3] = { 0, 0, 0 };
	SlicePlane->GetOrigin(PlaneOrigin);

	PlaneOrigin[0] += VolumeOrigin[0];
	PlaneOrigin[1] += VolumeOrigin[1];
	PlaneOrigin[2] += VolumeOrigin[2];

	SlicePlane->SetOrigin(PlaneOrigin);

	auto ResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();

	ResliceMapper->SetSlicePlane(SlicePlane);

	ResliceMapper->SetInputData(VolumeData);

	auto SliceProp = vtkImageSlice::New();

	SliceProp->SetMapper(ResliceMapper);

	if (ImageProperty != nullptr)
	{
		SliceProp->SetProperty(ImageProperty);
		ImageProperty->Delete();
	}
	else
	{
		double DataRange[2] = { 0, 0 };

		VolumeData->GetScalarRange(DataRange);

		auto tempImageProperty = this->CreateDefaultImageProperty(DataRange);

		SliceProp->SetProperty(tempImageProperty);
		tempImageProperty->Delete();
	}

	// do not use VolumeData.Delete() here

	return SliceProp;
}


vtkImageProperty* QVtkFigure::CreateDefaultImageProperty(const double DataRange[2])
{	
	auto ImageProperty = vtkImageProperty::New();

	if (DataRange[1] < DataRange[0])
	{
		qWarning() << "DataRange[1] < DataRange[0] @ CreateDefaultImageProperty";

		return ImageProperty; // keep it simple, nullptr will lead to crash
	}

	auto Lut = vtkSmartPointer<vtkLookupTable>::New();

	Lut->SetIndexedLookup(false);

	Lut->SetTableRange(DataRange[0], DataRange[1]);
	Lut->SetSaturationRange(0, 0);
	Lut->SetHueRange(0, 0);
	Lut->SetValueRange(0, 1);
	Lut->SetRampToLinear();

	Lut->Build();

	ImageProperty->SetOpacity(1);

	ImageProperty->SetLookupTable(Lut);

	ImageProperty->UseLookupTableScalarRangeOn();

	return ImageProperty;
}

//======================================= Show Mesh ==============================================================
quint64 QVtkFigure::ShowPloyMesh(vtkPolyData* MeshData)
{
	auto Prop = this->CreatePloyMeshProp(MeshData);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Mesh";

	PropInfo.NameOnMenu = "Mesh(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	PropInfo.DataSource = MeshData;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreatePloyMeshProp(vtkPolyData* MeshData)
{
	auto MeshMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	MeshMapper->SetInputData(MeshData);
	
	MeshData->Delete();

	auto MeshProp = vtkActor::New();
	MeshProp->SetMapper(MeshMapper);

	double ColorValue[3] = { 1, 1, 1 }; //white default;

	auto FiledPtr = MeshData->GetFieldData();

	auto Num = FiledPtr->GetNumberOfArrays();
	if (Num > 0)
	{
		auto Name = QString(FiledPtr->GetArrayName(0));
		if (Name == "Color")
		{
			FiledPtr->GetArray(0)->GetTuple(0, ColorValue);
		}
	}

	MeshProp->GetProperty()->SetColor(ColorValue);

	return MeshProp;
}
