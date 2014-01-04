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
#include <QVTKWidget.h> 

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
	auto StartTime = m_time.elapsed();
	auto EndTime = StartTime;

	while (true)
	{
		EndTime = m_time.elapsed();

		if (EndTime - StartTime > 1)
		{
			break;
		}
	}

	return (quint64)EndTime;

	/*  only 1 handler per sec 
	auto StartTime = std::time(nullptr); // time_t to double -> overflow ?

	auto EndTime = StartTime;

	while (true)
	{
		EndTime = std::time(nullptr);

		if (EndTime - StartTime > 1)
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

		m_PropRecord[PropInfo.Handle] = PropInfo;

		this->m_Renderer->AddViewProp(PropInfo.Prop);

		m_MainWindow->AddPropMenu(this, &PropInfo);

		// when adding first component: automatically reset view
		if (IsFirst)
		{
			m_QVtkWidget->GetRenderWindow()->Render();
			m_Renderer->ResetCamera();
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

		PropInfo.DataSource->Delete();

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

		this->Refresh();
	}

}

//===================================== Plot Point ==============================================================

quint64 QVtkFigure::PlotPoint(vtkPoints* points)
{
	auto Prop = this->CreatePointProp(points);

	PropInfomration PropInfo;

	PropInfo.Name = "Points";

	PropInfo.Handle = this->GeneratePropHandle();

	PropInfo.Prop = Prop;

	PropInfo.DataSource = points;

	this->AddProp(PropInfo);

	return PropInfo.Handle;
}

vtkProp* QVtkFigure::CreatePointProp(vtkPoints *points)
{
	std::cout << "vtk points: " << points << std::endl;

	auto InputData = vtkSmartPointer<vtkPolyData>::New();
	InputData->SetPoints(points);

	// draw spheres at the points
	auto Sphere = vtkSmartPointer<vtkSphereSource>::New();

	auto Glyphs = vtkSmartPointer<vtkGlyph3D>::New();

	Glyphs->SetInputData(0, InputData);

	Glyphs->SetInputConnection(1, Sphere->GetOutputPort());

	Glyphs->ScalingOff();
	Glyphs->OrientOn();
	Glyphs->ClampingOff();
	Glyphs->SetVectorModeToUseVector();
	Glyphs->SetIndexModeToOff();

	// set up mapper that shows shperes at points
	auto GlyphsMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	GlyphsMapper->SetInputConnection(Glyphs->GetOutputPort());

	auto GlyphActor = vtkActor::New();
	GlyphActor->SetMapper(GlyphsMapper);

	//upcast to vktProp*, dynamic_cast is not necessary
	return GlyphActor;  

	/* wrong 
	auto GlyphActor = vtkSmartPointer<vtkActor>::New();
	GlyphActor->SetMapper(GlyphsMapper);
	vtkProp* Prop = GlyphActor;
	return GlyphActor;
	// crash here
	*/
}
//===================================== End of Plot Point ==============================================================

//======================================= Show Image ===================================================================
quint64 QVtkFigure::ShowImage(vtkImageData* ImageData)
{
	return 0;
}

vtkProp* QVtkFigure::CreateImageProp(vtkImageData* ImageData)
{
	return nullptr;
}
//===================================== End of Show Image ==============================================================
