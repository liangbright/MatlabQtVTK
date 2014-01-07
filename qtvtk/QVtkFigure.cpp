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
#include <vtkRenderer.h> 
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkConeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkImageWriter.h>
#include <vtkPNGWriter.h>
#include <vtkJPEGWriter.h>
#include <vtkWindowToImageFilter.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkImageProperty.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeMapper.h>
#include <vtkVolumeTextureMapper3D.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkDataSetMapper.h>
#include <vtkDelaunay3D.h>
#include <vtkGeometryFilter.h>
#include <vtkLookupTable.h>

#include "QVtkFigureMainWindow.h"
#include "QVtkFigure.h"

QVtkFigure::QVtkFigure(quint64 Handle)
{
	m_Handle = Handle;

	m_MainWindow = new QVtkFigureMainWindow();
	
	m_QVtkWidget = new QVTKWidget();

	m_MainWindow->setCentralWidget(m_QVtkWidget);

	m_MainWindow->resize(512, 512);

	m_QVtkWidget->resize(m_MainWindow->size());

	m_QVtkWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_Renderer = vtkRenderer::New();

	m_QVtkWidget->GetRenderWindow()->AddRenderer(m_Renderer);

	m_MainWindow->CreateMenus(this);

	connect(m_MainWindow, &QVtkFigureMainWindow::UserCloseMainWindow, this, &QVtkFigure::Close);

	m_time.start();

	m_PropCounter = 0;

	this->Show();
}

QVtkFigure::~QVtkFigure()
{
	m_Renderer->Delete();

	m_MainWindow->deleteLater();

	// Qt memory management cleans these up:
	//delete m_QVtkWidget;	
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
	emit(QVtkFigureClosed());
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
	m_PropCounter = m_PropCounter + 1;

	return m_PropCounter;

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

//===================================== Plot Point =====================================
// Input:
//   Points: 
// Output
//   PropInfo.Handle : 0 if something goes wrong; >0 is good
//---------------------------------------------------------------------------------------
quint64 QVtkFigure::PlotPoint(vtkPoints* Point)
{
	auto Prop = this->CreatePointProp(Point);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Points(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreatePointProp(vtkPoints *Point)
{
	qDebug() << "vtk points: " << Point;

	qDebug() << "Point refrence counter is " << Point->GetReferenceCount(); //1

	auto PolyData = vtkPolyData::New(); // reference counter of PolyData is 1
	qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//1
		
	PolyData->SetPoints(Point);// increase the reference counter of Point
	qDebug() << "Point refrence counter is " << Point->GetReferenceCount();//2

	auto Glyph = vtkSmartPointer<vtkGlyph3D>::New();

	Glyph->SetInputData(0, PolyData);//increase the reference counter of PolyData

	qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//3
	PolyData->Delete();//decrease the reference counter of PolyData
	qDebug() << "PolyData refrence counter is " << PolyData->GetReferenceCount();//2

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
	Point->Delete(); 
	qDebug() << "Point refrence counter is " << Point->GetReferenceCount();//1

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
quint64 QVtkFigure::ShowVolume(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty, QString RenderMethord)
{
	auto Prop = this->CreateVolumeProp(VolumeData, VolumeProperty, RenderMethord);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Volume(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	//PropInfo.DataSource = VolumeData;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}

vtkProp* QVtkFigure::CreateVolumeProp(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty, \
	                                  QString RenderMethord)
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

	VolumeProp->SetProperty(VolumeProperty);

	//VolumeProp->Modified();

	VolumeMapper->Delete();
	VolumeProperty->Delete();
	VolumeData->Delete();

	return VolumeProp;
}

QString QVtkFigure::GetDefaultRenderMethod()
{
	return QString("RayCast");
}

vtkVolumeProperty* QVtkFigure::GetDefaultVolumeProperty(double DataRange[2])
{
	auto VolumeProperty = vtkVolumeProperty::New();

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

	double step = (dataDiff / 10.0);
	for (double i = DataRange[0]; i <= DataRange[1]; i += step)
	{
		auto *color = ColorTransferFunction->GetColor(i);
		qDebug("auto color: %.2f -> %.2f %.2f %.2f", i, color[0], color[1], color[2]);
	}
	
	// create default opacity lut, constant opacities over whole data range
	double opacity = 0.05;
	
	OpacityTransferFunction->AddPoint(DataRange[0], opacity);
	OpacityTransferFunction->AddPoint(DataRange[1], opacity);

	VolumeProperty->SetColor(ColorTransferFunction);
	
	VolumeProperty->SetScalarOpacity(OpacityTransferFunction);

	return VolumeProperty;
}


//======================================= Show Mesh ==============================================================
quint64 QVtkFigure::ShowPloyMesh(vtkPolyData* MeshData, QString Color)
{
	auto Prop = this->CreatePloyMeshProp(MeshData, Color);

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Mesh(" + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = Prop;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreatePloyMeshProp(vtkPolyData* MeshData, QString Color)
{
	auto MeshMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	auto p = new int[3];

/*	// read and set labels, if available

	vtkDataArray* labels;

		// set labels as scalar data to the points
		MeshData->GetPointData()->SetScalars(labels);
		
		// map scalars to colors
		SurfMapper->ScalarVisibilityOn();
		SurfMapper->SetScalarRange(labels->GetRange());

	// build and set lut
	if (colorReader->GetSerializer() != NULL) 
	{
		vtkColorTransferFunction *colorLut = colorReader->GetSerializer()->GetColorTransferFunction();
		SurfMapper->SetLookupTable(colorLut);
	}
	else 
	{
		vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
		lut->SetHueRange(0.6667, 0);
		SurfMapper->SetLookupTable(lut);
	}

*/
	MeshMapper->SetInputData(MeshData);
	
	auto MeshProp = vtkActor::New();
	MeshProp->SetMapper(MeshMapper);

	MeshData->Delete();

	return MeshProp;
}