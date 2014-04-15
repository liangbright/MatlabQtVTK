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
#include <vtkStringArray.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkGlyph3D.h>
#include <vtkTensorGlyph.h>
#include <vtkPolyDataNormals.h>
#include <vtkConeSource.h>
#include <vtkLineSource.h>
#include <vtkArrowSource.h>
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
#include <vtkDelaunay3D.h>
#include <vtkGeometryFilter.h>
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


bool QVtkFigure::RemoveProp(quint64 PropHandle)
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

		return true;
	}

	return false;
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

//===================================== Show Axes =====================================
//
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

	PropInfo.NameOnMenu = "Axes (Handle = " + QString::number(PropInfo.Handle) + ")";

	PropInfo.Prop = axes;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


//------------------ Get PropName ----------------------------------------------------------
//
QString QVtkFigure::ExtractPropName(vtkDataObject* DataObject)
{
	QString PropName;

	auto FiledPtr = DataObject->GetFieldData();

    auto Num = FiledPtr->GetNumberOfArrays();
    for (int i = 0; i<Num; ++i)
    {
	    auto Name = QString(FiledPtr->GetArrayName(i));
		if (Name == "PropName")
		{
			auto StrPtr = static_cast<vtkStringArray*>(FiledPtr->GetAbstractArray(i));
			PropName = StrPtr->GetValue(0);
			break;
		}
	}

	return PropName;
}

//------------------ Get PropColor ----------------------------------------------------------
//
bool QVtkFigure::ExtractPropColor(vtkDataObject* DataObject, double Color[3])
{
	auto FiledPtr = DataObject->GetFieldData();

	auto Num = FiledPtr->GetNumberOfArrays();
	for (int i = 0; i<Num; ++i)
	{
		auto Name = QString(FiledPtr->GetArrayName(i));
		if (Name == "PropColor")
		{
			FiledPtr->GetArray(i)->GetTuple(0, Color);
			return true;
		}
	}

	return false;
}

//===================================== Plot Point =====================================
// Input:
//   Points: 
// Output
//   PropInfo.Handle : 0 if something goes wrong; >0 is good
//---------------------------------------------------------------------------------------
quint64 QVtkFigure::PlotPoint(vtkPolyData* PointData)
{
	auto Prop = this->CreatePointProp(PointData);
	//--------------------------------------------------------------------------------
	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Point";

	PropInfo.NameOnMenu = this->ExtractPropName(PointData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Points (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = PointData;
	//--------------------------------------------------------------------------------

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreatePointProp(vtkPolyData* PointData)
{
	auto Glyph = vtkSmartPointer<vtkGlyph3D>::New();

	Glyph->SetInputData(0, PointData);//increase the reference counter of PointData
	
	PointData->Delete();//decrease the reference counter of PointData

	// draw spheres at the points
	auto Sphere = vtkSmartPointer<vtkSphereSource>::New();

	Glyph->SetInputConnection(1, Sphere->GetOutputPort());

	Glyph->ScalingOff();
	Glyph->OrientOn();
	Glyph->ClampingOff();
	Glyph->SetVectorModeToUseVector();
	Glyph->SetIndexModeToOff();

	auto GlyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	GlyphMapper->SetInputConnection(Glyph->GetOutputPort());

	auto GlyphActor = vtkActor::New(); //refrence counter of PointActor is 1
	GlyphActor->SetMapper(GlyphMapper);

	//---------------------- set color -------------------------------------------
	double ColorValue[3] = { 1, 1, 1 }; //white default;
	this->ExtractPropColor(PointData, ColorValue);


	GlyphActor->GetProperty()->SetColor(ColorValue);
	//-----------------------------------------------------------------

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

//===================================== Plot Line =====================================
// Input:
//   Points: 
// Output
//   PropInfo.Handle : 0 if something goes wrong; >0 is good
//---------------------------------------------------------------------------------------
quint64 QVtkFigure::PlotLine(vtkPolyData* LineData)
{
	auto Prop = this->CreateLineProp(LineData);
	//--------------------------------------------------------------------------------
	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Line";

	PropInfo.NameOnMenu = this->ExtractPropName(LineData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Points (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = LineData;
	//--------------------------------------------------------------------------------

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreateLineProp(vtkPolyData* LineData)
{
	auto LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	LineMapper->SetInputData(LineData);

	LineData->Delete();

	auto LineProp = vtkActor::New();
	LineProp->SetMapper(LineMapper);

	//----------------------- set color ----------------------------------
	double ColorValue[3] = { 1, 1, 1 }; //white default;
	this->ExtractPropColor(LineData, ColorValue);

	LineProp->GetProperty()->SetColor(ColorValue);
	//---------------------------------------------------------------------

	return LineProp;
}

//======================================= Plot Vector ==============================================================
//
quint64 QVtkFigure::PlotVector(vtkPolyData* VectorData)
{
	auto Prop = this->CreateVectorProp(VectorData);
	//--------------------------------------------------------------------------------
	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Vector";

	PropInfo.NameOnMenu = this->ExtractPropName(VectorData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Vectors (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = VectorData;
	//--------------------------------------------------------------------------------

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreateVectorProp(vtkPolyData* VectorData)
{
	auto arrow = vtkSmartPointer<vtkArrowSource>::New();

	auto glyphs = vtkSmartPointer<vtkGlyph3D>::New();

	glyphs->SetInputData(0, VectorData);

	VectorData->Delete();

	glyphs->SetInputConnection(1, arrow->GetOutputPort());

	glyphs->ScalingOn();
	glyphs->SetScaleModeToScaleByVector();

	glyphs->OrientOn();
	glyphs->ClampingOff();
	glyphs->SetVectorModeToUseVector();
	glyphs->SetIndexModeToOff();

	auto glyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	glyphMapper->SetInputConnection(glyphs->GetOutputPort());

	auto glyphActor = vtkActor::New();

	glyphActor->SetMapper(glyphMapper);

	//----------------------- set color ----------------------------------
	double ColorValue[3] = { 1, 1, 1 }; //white default;
	this->ExtractPropColor(VectorData, ColorValue);

	glyphActor->GetProperty()->SetColor(ColorValue);
	//---------------------------------------------------------------------

	return glyphActor;
}

//======================================= Plot Tensor ==============================================================
//
quint64 QVtkFigure::PlotTensor(vtkPolyData* TensorData)
{
	auto Prop = this->CreateVectorProp(TensorData);
	//--------------------------------------------------------------------------------
	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Tensor";

	PropInfo.NameOnMenu = this->ExtractPropName(TensorData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Tensors (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = TensorData;
	//--------------------------------------------------------------------------------

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}


vtkProp* QVtkFigure::CreateTensorProp(vtkPolyData* TensorData)
{
	auto source = vtkSmartPointer<vtkSphereSource>::New();

	auto glyphs = vtkSmartPointer<vtkTensorGlyph>::New();

	glyphs->SetInputData(0, TensorData);

	TensorData->Delete();

	glyphs->SetInputConnection(1, source->GetOutputPort());

	glyphs->ColorGlyphsOn();

	auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();

	normals->SetInputConnection(glyphs->GetOutputPort());

	auto glyphMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

	glyphMapper->SetInputConnection(normals->GetOutputPort());

	auto glyphActor = vtkActor::New();

	glyphActor->SetMapper(glyphMapper);

	//----------------------- set color ----------------------------------
	double ColorValue[3] = { 1, 1, 1 }; //white default;
	this->ExtractPropColor(TensorData, ColorValue);

	glyphActor->GetProperty()->SetColor(ColorValue);
	//---------------------------------------------------------------------

	return glyphActor;
}

//======================================= Show Mesh ==============================================================
quint64 QVtkFigure::ShowPloyMesh(vtkPolyData* MeshData)
{
	auto Prop = this->CreatePloyMeshProp(MeshData);
	//--------------------------------------------------------------------------------

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Mesh";

	PropInfo.NameOnMenu = this->ExtractPropName(MeshData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Mesh (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = MeshData;
	//--------------------------------------------------------------------------------

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

	//----------------------- set color ------------------------------------
	double ColorValue[3] = { 1, 1, 1 }; //white default;
	this->ExtractPropColor(MeshData, ColorValue);

	MeshProp->GetProperty()->SetColor(ColorValue);

	return MeshProp;
}

//======================================= Show Volume Image ==============================================================
quint64 QVtkFigure::ShowVolume(vtkImageData* VolumeData, vtkVolumeProperty* VolumeProperty)
{
	auto Prop = this->CreateVolumeProp(VolumeData, VolumeProperty);
	//--------------------------------------------------------------------------------
	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "Volume";

	PropInfo.NameOnMenu = this->ExtractPropName(VolumeData);
	if (PropInfo.NameOnMenu == "")
	{
		PropInfo.NameOnMenu.append("Volume (Handle = " + QString::number(PropInfo.Handle) + ")");
	}
	else
	{
		PropInfo.NameOnMenu.append(" (Handle = " + QString::number(PropInfo.Handle) + ")");
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = VolumeData;
	//--------------------------------------------------------------------------------

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
		qDebug("GPU RayCasting is selected");
		VolumeMapper = GpuRcMapper;
	}
	// no fast GPU raycasting available.
	else
	{
		GpuRcMapper->Delete();

		qDebug("software RayCasting is selected");
		//vtkVolumeRayCastCompositeFunction *compositeFunction = vtkVolumeRayCastCompositeFunction::New();

		auto *RcMapper = vtkFixedPointVolumeRayCastMapper::New();

		RcMapper->SetBlendModeToComposite();

		//raycastVolumeMapper->SetVolumeRayCastFunction(compositeFunction);
		//compositeFunction->Delete();
		//raycastVolumeMapper->AutoAdjustSampleDistancesOn();

		//Debug() << "sample distance" << raycastVolumeMapper->GetSampleDistance();

		VolumeMapper = RcMapper;
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

        //auto tempVolumeProperty = this->CreateDefaultVolumeProperty_Color(DataRange);

		auto tempVolumeProperty = this->CreateDefaultVolumeProperty_Gray(DataRange);

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


vtkVolumeProperty* QVtkFigure::CreateDefaultVolumeProperty_Color(const double DataRange[2])
{
	auto VolumeProperty = vtkVolumeProperty::New();

	if (DataRange[1] < DataRange[0])
	{
		qWarning() << "DataRange[1] < DataRange[0] @ CreateDefaultVolumeProperty_Color";

		return VolumeProperty; // keep it simple, nullptr will lead to crash
	}

	auto ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

	auto OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

	qDebug("DataRange: %.2f - %.2f", DataRange[0], DataRange[1]);
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
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*0.875), 1.0, 0.5, 0.0);
	ColorTransferFunction->AddRGBPoint(dataMin + (dataDiff*1.000), 1.0, 0.0, 0.0);

    double max_opacity = 1.0;

    double step = (DataRange[1] - DataRange[0]) / 100;
	for (double level = DataRange[0]; level < DataRange[1]; level += step)
	{
        double opacity = max_opacity*(level - DataRange[0]) / dataDiff;

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


vtkVolumeProperty* QVtkFigure::CreateDefaultVolumeProperty_Gray(const double DataRange[2])
{
    auto VolumeProperty = vtkVolumeProperty::New();

    if (DataRange[1] < DataRange[0])
    {
        qWarning() << "DataRange[1] < DataRange[0] @ CreateDefaultVolumeProperty_Gray";

        return VolumeProperty; // keep it simple, nullptr will lead to crash
    }

    auto ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

    ColorTransferFunction->AddRGBPoint(DataRange[0], 0.0, 0.0, 0.0);
    ColorTransferFunction->AddRGBPoint(DataRange[1], 1.0, 1.0, 1.0);


    auto OpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

    double dataDiff = (DataRange[1] - DataRange[0]);

    double max_opacity = 1.0;

    double step = dataDiff / 100;
    for (double level = DataRange[0]; level < DataRange[1]; level += step)
    {
        double opacity = max_opacity*(level - DataRange[0]) / dataDiff;

        OpacityTransferFunction->AddPoint(level, opacity);
    }
    
    //constant opacities over whole data range
    //double opacity = 1;
    //OpacityTransferFunction->AddPoint(DataRange[0], opacity);
    //OpacityTransferFunction->AddPoint(DataRange[1], opacity);

    VolumeProperty->SetColor(ColorTransferFunction);

    VolumeProperty->SetScalarOpacity(OpacityTransferFunction);

    return VolumeProperty;
}


quint64 QVtkFigure::ShowSliceOfVolume(quint64 VolumePropHandle, vtkPlane* SlicePlane, QString SliceName, 
	                                  vtkImageProperty* ImageProperty)
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

	//--------------------------------------------------------------------------

	PropInfomration PropInfo;

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Name = "SliceOfVolume";

	auto VolumePropName = this->ExtractPropName(VolumeData);
	if (VolumePropName == "")
	{
		VolumePropName = "Volume";
	}

	if (SliceName == "")
	{
		PropInfo.NameOnMenu = "Slice Of " + VolumePropName + " (Handle = " + QString::number(PropInfo.Handle) + ")";
	}
	else
	{
		PropInfo.NameOnMenu = SliceName + " Of " + VolumePropName + " (Handle = " + QString::number(PropInfo.Handle) + ")";
	}

	PropInfo.Prop = Prop;

	PropInfo.DataSource = VolumeData;
	//--------------------------------------------------------------------------------

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

	SlicePlane->Delete();

	ResliceMapper->SetInputData(VolumeData);

	// do not use VolumeData.Delete() here

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
