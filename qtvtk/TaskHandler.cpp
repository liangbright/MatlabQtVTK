#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QDebug>
#include <QDir>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

#include <ctime>

#include "TaskHandler.h"

TaskHandler::TaskHandler()
{
	this->CreateMatlabCommandTranslator();

	m_time.start();
}

TaskHandler::~TaskHandler()
{
}


void TaskHandler::CreateMatlabCommandTranslator()
{
	QString Commmand;

	Commmand = "vtkfigure";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkfigure);

	Commmand = "vtkplotpoint";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplotpoint);
		
	Commmand = "vtkshowvolume";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowvolume);

	Commmand = "vtkshowmesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowmesh);

	Commmand = "vtkdeleteprop";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkdeleteprop);
	
}

void TaskHandler::CloseQVtkFigure()
{
	auto Figure = dynamic_cast<QVtkFigure*>(QObject::sender());
	if (Figure == nullptr)
	{
		return;
	}

	auto FigureHandle = Figure->GetHandle();

	auto it = m_FigureRecord.find(FigureHandle);
	if (it != m_FigureRecord.end())
	{
		it->second.release()->deleteLater();
		m_FigureRecord.erase(it);
	}
}


bool TaskHandler::WriteTaskFailureInfo(const TaskInformation& TaskInfo, QString ResultFileName, QString FailureInfo)
{
	QString tempName = TaskInfo.Path + TaskInfo.FolderName + "/~" + ResultFileName;

	QFile ResultFile(tempName);

	if (!ResultFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file to save result");
		return false;
	}

	QJsonObject ResultObject;

	ResultObject["IsSuccess"] = QString("no");

	ResultObject["FigureHandle"] = QString("");

	ResultObject["PropHandle"] = QString("");

	ResultObject["FailureInfo"] = FailureInfo;

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());
	ResultFile.close();

	ResultFile.rename(TaskInfo.Path + TaskInfo.FolderName + "/" + ResultFileName);

	return true;
}


bool TaskHandler::run_vtkfigure(const TaskInformation& Task)
{
	qDebug() << "run_vtkfigure";

	QFile TaskFile(Task.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open task file.");
		return false;
	}

	QByteArray TaskContent = TaskFile.readAll();
	QJsonDocument TaskDoc(QJsonDocument::fromJson(TaskContent));
	QJsonObject TaskObject = TaskDoc.object();

	//-------------------- Read some info ----------------------------------------//
	QString ResultFileName;
	auto it = TaskObject.find("ResultFileName");
	if (it != TaskObject.end())
	{
		ResultFileName = it.value().toString();
	}
	else
	{
		qWarning("ResultFileName is unknown");
		return false;
	}

	// new figure --------------------------------------------------------------//
	auto FigureHandle = this->GenerateFigureHandle();

	auto Figure = std::unique_ptr<QVtkFigure>(new QVtkFigure(FigureHandle));

	connect(Figure.get(), &QVtkFigure::QVtkFigureClosed, this, &TaskHandler::CloseQVtkFigure);

	Figure->Show();

	m_FigureRecord[FigureHandle] = std::move(Figure);
	//---------------------- Write Result ----------------------------------------//

	QString tempName = Task.Path + Task.FolderName + "/~" + ResultFileName;

	QFile ResultFile(tempName);

	if (!ResultFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file to save result");
		return false;
	}

	QJsonObject ResultObject;

	ResultObject["IsSuccess"] = QString("yes");

	ResultObject["FigureHandle"] = QString::number(FigureHandle);

	ResultObject["PropHandle"] = QString("");

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());
	ResultFile.close();

	ResultFile.rename(Task.Path + Task.FolderName + "/" + ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkplotpoint(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkplotpoint";

	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open task file.");
		return false;
	}

	QByteArray TaskContent = TaskFile.readAll();
	QJsonDocument TaskDoc(QJsonDocument::fromJson(TaskContent));
	QJsonObject TaskObject = TaskDoc.object();

	//-------------------- Read some Information from Task.json ----------------------------------//
	QString ResultFileName;
	auto it = TaskObject.find("ResultFileName");
	if (it != TaskObject.end())
	{
		ResultFileName = it.value().toString();
	}
	else
	{
		qWarning("ResultFileName is unknown");
		return false;
	}

	//get FigureHandle
	quint64 FigureHandle = 0; // invalid handle
	it = TaskObject.find("FigureHandle");
	if (it != TaskObject.end())
	{
		FigureHandle = it.value().toString().toULongLong();
	}
	else
	{
		QString FailureInfo = "FigureHandle is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//check FigureHandle
	QVtkFigure* Figure = nullptr;
	auto it_Fig = m_FigureRecord.find(FigureHandle);
	if (it_Fig != m_FigureRecord.end())
	{
		Figure=it_Fig->second.get();
		if (Figure == nullptr)
		{
			QString FailureInfo = "FigureHandle is null in m_FigureRecord";			
			TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
			qWarning() << FailureInfo;
			return false;
		}
	}
	else
	{
		QString FailureInfo = "FigureHandle is invalid";		
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//get PointNum
	quint64 PointNum = 0;
	it = TaskObject.find("PointNum");
	if (it != TaskObject.end())
	{
		PointNum = it.value().toString().toULongLong();
	}
	else
	{
		QString FailureInfo = "PointNum is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get MatlabDataType
	QString DataType;
	it = TaskObject.find("PointDataType");
	if (it != TaskObject.end())
	{ DataType = it.value().toString();}
	else
	{
		QString FailureInfo = "PointDataType is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}
	
	// Get Color if it is specified in the file
	double PointColor[3] = { 1, 1, 1 }; // {R, G, B}; white
	bool IscolorOK = true;

	it = TaskObject.find("PointColor_R");
	if (it != TaskObject.end())
	{
		PointColor[0] = it.value().toString().toDouble();
	}
	else
	{ IscolorOK = false;}

	it = TaskObject.find("PointColor_G");
	if (it != TaskObject.end())
	{
		PointColor[1] = it.value().toString().toDouble();
	}
	else
	{ IscolorOK = false;}

	it = TaskObject.find("PointColor_B");
	if (it != TaskObject.end())
	{
		PointColor[2] = it.value().toString().toDouble();
	}
	else
	{ IscolorOK = false;}

	if (IscolorOK == false)
	{
		PointColor[0] = 1;
		PointColor[1] = 1;
		PointColor[2] = 1;
	}

	QString DataFileFullNameAndPath;
	it = TaskObject.find("PointDataFileName");
	if (it != TaskObject.end())
	{ 
		DataFileFullNameAndPath = TaskInfo.GetFullPath() + it.value().toString();
	}
	else
	{
		QString FailureInfo = "PointDataFileName is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}
	
	//--------------------- Get the data ---------------------------------------//

	vtkPoints* Point = nullptr;
	auto IsReadOK = ReadPointData(DataFileFullNameAndPath, PointNum, DataType, &Point);

	if (Point == nullptr)
	{
		QString FailureInfo = "Point Data is not loaded";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//---------------------- Plot Point ----------------------------------------//
	auto PropHandle = Figure->PlotPoint(Point);

	//---------------------- Write Result ----------------------------------------//
	QString tempName = TaskInfo.Path + TaskInfo.FolderName + "/~" + ResultFileName;

	QFile ResultFile(tempName);

	if (!ResultFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file to save result");
		return false;
	}

	QJsonObject ResultObject;

	ResultObject["IsSuccess"] = QString("yes");

	ResultObject["FigureHandle"] = QString::number(FigureHandle);

	ResultObject["PropHandle"] = QString::number(PropHandle);

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());
	ResultFile.close();

	ResultFile.rename(TaskInfo.Path + TaskInfo.FolderName + "/" + ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowvolume(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open task file.");
		return false;
	}
	//----------------------------------------------------------//
	QByteArray TaskContent = TaskFile.readAll();
	QJsonDocument TaskDoc(QJsonDocument::fromJson(TaskContent));
	QJsonObject TaskObject = TaskDoc.object();

	//-------------------- Read some Information from Task.json ----------------------------------//

	// get ResultFileName ----------------------------------------------------------
	QString ResultFileName;
	auto it = TaskObject.find("ResultFileName");
	if (it != TaskObject.end())
	{
		ResultFileName = it.value().toString();
	}
	else
	{
		qWarning("ResultFileName is unknown");
		return false;
	}

	//get FigureHandle ----------------------------------------------------------
	quint64 FigureHandle = 0; // invalid handle
	it = TaskObject.find("FigureHandle");
	if (it != TaskObject.end())
	{ FigureHandle = it.value().toString().toULongLong();}
	else
	{
		QString FailureInfo = "FigureHandle is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//check FigureHandle ----------------------------------------------------------
	QVtkFigure* Figure = nullptr;
	auto it_Fig = m_FigureRecord.find(FigureHandle);
	if (it_Fig != m_FigureRecord.end())
	{
		Figure = it_Fig->second.get();
		if (Figure == nullptr)
		{
			QString FailureInfo = "Figure is invalid";
			TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
			qWarning() << FailureInfo;
			return false;
		}
	}
	else
	{
		QString FailureInfo = "FigureHandle is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//get image size ----------------------------------------------------------
	int ImageSize[3] = { 0, 0, 0 };
	bool IsImageSizeOK = true;

	it = TaskObject.find("ImageSize_x");
	if (it != TaskObject.end())
	{
		ImageSize[0] = it.value().toString().toInt();
	}
	else
	{ IsImageSizeOK = false;}

	it = TaskObject.find("ImageSize_y");
	if (it != TaskObject.end())
	{
		ImageSize[1] = it.value().toString().toInt();
	}
	else
	{ IsImageSizeOK = false;}

	it = TaskObject.find("ImageSize_z");
	if (it != TaskObject.end())
	{
		ImageSize[2] = it.value().toString().toInt();
	}
	else
	{ IsImageSizeOK = false;}

	if (IsImageSizeOK == false)
	{
		QString FailureInfo = "ImageSize is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//get image origin ----------------------------------------------------------
	double Origin[3] = { 0.0, 0.0, 0.0 };
	bool IsOriginOK = true;

	it = TaskObject.find("Origin_x");	
	if (it != TaskObject.end())
	{
		Origin[0] = it.value().toString().toDouble();
	}
	else
	{ IsOriginOK = false;}

	it = TaskObject.find("Origin_y");
	if (it != TaskObject.end())
	{
		Origin[1] = it.value().toString().toDouble();
	}
	else
	{ IsOriginOK = false;}

	it = TaskObject.find("Origin_z");
	if (it != TaskObject.end())
	{
		Origin[2] = it.value().toString().toDouble();
	}
	else
	{ IsOriginOK = false;}

	if (IsOriginOK == false)
	{
		QString FailureInfo = "Origin is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get MatlabDataType ----------------------------------------------------------
	QString DataType;
	it = TaskObject.find("DataType");
	if (it != TaskObject.end())
	{ DataType = it.value().toString();}
	else
	{
		QString FailureInfo = "DataType is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get ImageDataFileName ----------------------------------------------------------
	QString DataFileFullNameAndPath;
	it = TaskObject.find("ImageDataFileName");
	if (it != TaskObject.end())
	{
		DataFileFullNameAndPath = TaskInfo.GetFullPath() + it.value().toString() + ".data";
	}
	else
	{
		QString FailureInfo = "ImageDataFileName is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get DataRange ----------------------------------------------------------
	double DataRange[2];
	it = TaskObject.find("DataRange_Min");
	if (it != TaskObject.end())
	{
		DataRange[0] = it.value().toDouble();
	}
	else
	{
		QString FailureInfo = "DataRange_Min is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	it = TaskObject.find("DataRange_Max");
	if (it != TaskObject.end())
	{
		DataRange[1] = it.value().toDouble();
	}
	else
	{
		QString FailureInfo = "DataRange_Max is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//--------------------- Get VolumeProperty ---------------------------------------
	auto VolumeProperty = Figure->GetDefaultVolumeProperty(DataRange);

	//--------------------- Get RenderMethod ---------------------------------------
	auto RenderMethod = Figure->GetDefaultRenderMethod();

	//--------------------- Get the data ---------------------------------------//
	vtkImageData* ImageData = nullptr;
	auto IsReadOK = ReadVolumeData(DataFileFullNameAndPath, ImageSize, DataType, &ImageData);
	if (ImageData == nullptr)
	{
		QString FailureInfo = "ImageData is not loaded";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}
	//---------------------- Show Image ----------------------------------------//
	auto PropHandle = Figure->ShowVolume(ImageData, VolumeProperty, RenderMethod);

	//---------------------- Write Result ----------------------------------------//
	QString tempName = TaskInfo.Path + TaskInfo.FolderName + "/~" + ResultFileName;
	QFile ResultFile(tempName);

	if (!ResultFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open save file.");
		return false;
	}

	QJsonObject ResultObject;

	ResultObject["IsSuccess"] = QString("yes");

	ResultObject["FigureHandle"] = QString::number(FigureHandle);

	ResultObject["PropHandle"] = QString::number(PropHandle);

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());

	ResultFile.rename(ResultFileName);

	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowmesh(const TaskInformation& Task)
{
	return true;
}


bool TaskHandler::run_vtkdeleteprop(const TaskInformation& Task)
{

	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::RunTask(const TaskInformation& TaskInfo)
{	
	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::ReadOnly)) 
	{
		qWarning("Couldn't open task file (*.json)");
		qWarning() << "Path" << TaskInfo.Path;
		qWarning() << "FolderName" << TaskInfo.FolderName;
		qWarning() << "FileName" << TaskInfo.GetFileName();

		return false;
	}

	QByteArray TaskContent = TaskFile.readAll();

	QJsonDocument TaskDoc(QJsonDocument::fromJson(TaskContent));

	QJsonObject TaskObject = TaskDoc.object();

	QString Command;
	auto it1 = TaskObject.find("Command");
	if (it1 != TaskObject.end())
	{
		Command = it1.value().toString();
	}
	else
	{
		qWarning("task file is invalid: no Command");
		return false;
	}
	
	qDebug() << "Matlab Command is " << Command;

	auto it2 = m_MatlabCommandTranslator.find(Command);
	if (it2 != m_MatlabCommandTranslator.end())
	{
		auto function = it2.value();
		return function(this, TaskInfo);
	}

	return false;
}


void TaskHandler::WriteExampleTaskFile(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file.");
		return;
	}

	QJsonObject TaskObject;

	TaskObject["Command"] = QString("vtkplotpoint");

	TaskObject["PointNum"] = QString("10");

	TaskObject["PointDataType"] = QString("vtkplotpoint");

	TaskObject["FigureHandle"] = QString::number(12345678901);

	TaskObject["PropHandle"] = QString::number(12345678902);

	QJsonDocument TaskDoc(TaskObject);

	TaskFile.write(TaskDoc.toJson());
}


void TaskHandler::ReadExampleTaskFile(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

	if (!TaskFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open task file.");
		return;
	}
	//----------------------------------------------------------//
	QByteArray TaskContent = TaskFile.readAll();
	QJsonDocument TaskDoc(QJsonDocument::fromJson(TaskContent));
	QJsonObject TaskObject = TaskDoc.object();

	//get Commmand
	QString Command;
	auto it = TaskObject.find("Command");
	if (it != TaskObject.end())
	{
		Command = it.value().toString();
	}

	//get FigureHandle
	quint64 FigureHandle = 1; // invalid handle
	it = TaskObject.find("FigureHandle");
	if (it != TaskObject.end())
	{
		FigureHandle = it.value().toString().toULongLong();
	}

	//get PropHandle
	quint64 PropHandle = 1; // invalid handle
	it = TaskObject.find("PropHandle");
	if (it != TaskObject.end())
	{
		PropHandle = it.value().toString().toULongLong();
	}

	qDebug() << "Command:" << Command;
	qDebug() << "FigureHandle:" << FigureHandle;
	qDebug() << "PropHandle:" << PropHandle;
}

//========================== Read point data file =================================================
//Input:
// DataFileFullNameAndPath, PointNum, MatlabDataType
//Output:
//  PointData: nullptr or pointer to data
bool TaskHandler::ReadPointData(QString DataFileFullNameAndPath, quint64 PointNum, QString MatlabDataType, \
	                            vtkPoints** PointData)
{
	//---------------------------------------------------------------------------------------//
	// defalut value
	*PointData = nullptr;
	//---------------------------------------------------------------------------------------//

	QFile DataFile(DataFileFullNameAndPath);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open data file:" << DataFileFullNameAndPath;

		return false;
	}
	//---------------------------------------------------------------------------------------//
	qint64 BypesofDataFile = DataFile.size();

	qint64 BypesofPointData = 0;

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		BypesofPointData = qint64(PointNum*3*8); // 3d point
	}
	else if (MatlabDataType == m_MatlabDataTypeList.Single)
	{
		BypesofPointData = qint64(PointNum*3*4);
	}
	else
	{
		BypesofPointData = 0;
	}

	if (BypesofDataFile != BypesofPointData)
	{
		qWarning("Data file size is not equal to point-set size");
		return false;
	}
	//---------------------------------------------------------------------------------------//
	auto Point = vtkPoints::New();
	
	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	Point->SetDataType(VtkDataType);

	Point->SetNumberOfPoints(PointNum);

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		double pos[3];

		for (quint64 i = 0; i < PointNum; ++i)
		{
			auto BypesofPos = DataFile.read((char *)pos, 24);

			if (BypesofPos != 24)
			{
				Point->Delete();
				return false;
			}

			Point->InsertPoint(i, pos);
		}
	}
	else if (MatlabDataType == m_MatlabDataTypeList.Single)
	{
		float pos[3];

		for (quint64 i = 0; i < PointNum; ++i)
		{
			auto BypesofPos = DataFile.read((char *)pos, 12);

			if (BypesofPos != 12)
			{
				Point->Delete();
				return false;
			}

			Point->InsertPoint(i, pos);
		}
	}

	Point->Modified();

	*PointData = Point;

	return true;
}


bool TaskHandler::ReadMeshData(QString PathAndFileName, quint64 ElementNum, QString MatlabDataType, \
	                           vtkPolyData** PolyData)
{
	return nullptr;
}


bool TaskHandler::ReadVolumeData(QString DataFileFullNameAndPath, int ImageSize[3], QString MatlabDataType, \
	                             vtkImageData** ImageData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	*ImageData = nullptr;
	//---------------------------------------------------------------------------------------//

	QFile DataFile(DataFileFullNameAndPath);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open data file.");

		return false;
	}

	//---------------------------------------------------------------------------------------//
	qint64 BypesofDataFile = DataFile.size();

	auto PixelNum = double(ImageSize[0]) * double(ImageSize[1]) * double(ImageSize[2]);

	qint64 BypesofImageData = 0;

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		BypesofImageData = qint64(PixelNum * 8);
	}
	else if (MatlabDataType == m_MatlabDataTypeList.Single)
	{
		BypesofImageData = qint64(PixelNum * 4);
	}
	else
	{
		BypesofImageData = 0;
	}

	if (BypesofDataFile != BypesofImageData)
	{
		qWarning("Data file size is not equal to image size");
		return false;
	}
	//---------------------------------------------------------------------------------------//

	auto Image = vtkImageData::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	Image->SetScalarType(VtkDataType, Image->GetInformation());

	Image->SetNumberOfScalarComponents(1, Image->GetInformation());

	qDebug() << "image data for type: " << Image->GetScalarTypeAsString();

	Image->SetDimensions(ImageSize[0], ImageSize[1], ImageSize[2]);

	int dims[3];
	Image->GetDimensions(dims);
	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	Image->AllocateScalars(Image->GetInformation());

	qDebug() << "image data memory size: " << Image->GetActualMemorySize() * 1024L;

	Image->GetDimensions(dims);
	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	qDebug() << "components:" << Image->GetNumberOfScalarComponents();
	qDebug() << "scalar size:" << Image->GetScalarSize();

	void *dataPtr = Image->GetScalarPointer();

	auto BypesofOutput = DataFile.read((char *)dataPtr, BypesofImageData);

	if (BypesofDataFile != BypesofImageData)
	{
		qWarning("Data file size is not equal to image size");
		Image->Delete();
		return false;
	}

	Image->Modified();

	*ImageData = Image;

	return true;
}


VtkDataTypeEnum TaskHandler::MapMatlabDataTypeToVtkDataType(QString MatlabDataType)
{
	if (MatlabDataType == m_MatlabDataTypeList.Double)
    {
	    return VtkDataTypeEnum::VALUE_DOUBLE;
	}
    else if (MatlabDataType == m_MatlabDataTypeList.Single)
    {
	    return VtkDataTypeEnum::VALUE_FLOAT;
    }
    else if (MatlabDataType == m_MatlabDataTypeList.Int8) 
    {
		return VtkDataTypeEnum::VALUE_CHAR;
    }
    else if (MatlabDataType == m_MatlabDataTypeList.Int32)
    {
		return VtkDataTypeEnum::VALUE_INTEGER;
    }
	else if (MatlabDataType == m_MatlabDataTypeList.Int64)
	{
		return VtkDataTypeEnum::VALUE_LONG;
	}
	else
	{
		return VtkDataTypeEnum::VALUE_UNKNOWN;
	}

}


quint64 TaskHandler::GenerateFigureHandle()
{
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
