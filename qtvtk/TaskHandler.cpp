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
}

TaskHandler::~TaskHandler()
{
}


void TaskHandler::CreateMatlabCommandTranslator()
{
	QString Commmand;

	Commmand = "vtkplotpoint";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplotpoint);
		
	Commmand = "vtkshowimage";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowimage);

	Commmand = "vtkshowmesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowmesh);

	Commmand = "vtkdeleteprop";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkdeleteprop);
	
}


bool TaskHandler::run_vtkplotpoint(QString Path, QString TaskFileName)
{
	qDebug() << "run_vtkplotpoint";

	QFile TaskFile(Path+TaskFileName + ".json");

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

	//get FigureHandle
	quint64 FigureHandle = 0; // invalid handle
	auto it = TaskObject.find("FigureHandle");
	if (it != TaskObject.end())
	{
		FigureHandle = it.value().toString().toULongLong();
	}
	else
	{
		qWarning("FigureHandle is unknown");
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
			qWarning("FigureHandle is null in m_FigureRecord");
			return false;
		}
	}
	else
	{
		qWarning("FigureHandle is invalid");
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
		qWarning("PointNum is unknown");
		return false;
	}

	// get MatlabDataType
	QString DataType;
	it = TaskObject.find("DataType");
	if (it != TaskObject.end())
	{ DataType = it.value().toString();}
	else
	{
		qWarning("DataType is unknown");
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
	{ DataFileFullNameAndPath = Path + it.value().toString() + ".data";}
	else
	{
		qWarning("PointDataFileName is unknown");
		return false;
	}

	QString ResultFileFullName;
	it = TaskObject.find("ResultFileName");
	if (it != TaskObject.end())
	{ ResultFileFullName = it.value().toString() + ".json";}
	else
	{
		qWarning("ResultFileName is unknown");
		return false;
	}

	//--------------------- Get the data ---------------------------------------//
	auto Point = ReadPointData(DataFileFullNameAndPath, PointNum, DataType);

	//---------------------- Plot Point ----------------------------------------//
	auto PropHandle = Figure->PlotPoint(Point);

	//---------------------- Write Result ----------------------------------------//
	QString tempName = Path + "~temp.json";

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

	ResultFile.rename(ResultFileFullName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowimage(QString Path, QString TaskFileName)
{
	QFile TaskFile(Path + TaskFileName + ".json");

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

	//get FigureHandle
	quint64 FigureHandle = 0; // invalid handle
	auto it = TaskObject.find("FigureHandle");
	if (it != TaskObject.end())
	{ FigureHandle = it.value().toString().toULongLong();}
	else
	{
		qWarning("FigureHandle is unknown");
		return false;
	}

	//check FigureHandle
	QVtkFigure* Figure = nullptr;
	auto it_Fig = m_FigureRecord.find(FigureHandle);
	if (it_Fig != m_FigureRecord.end())
	{
		Figure = it_Fig->second.get();
		if (Figure == nullptr)
		{
			qWarning("FigureHandle is null in m_FigureRecord");
			return false;
		}
	}
	else
	{
		qWarning("FigureHandle is invalid");
		return false;
	}

	//get image size
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
		qWarning("ImageSize is invalid");
		return false;
	}

	//get image origin
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
		qWarning("Origin is invalid");
		return false;
	}

	// get MatlabDataType
	QString DataType;
	it = TaskObject.find("DataType");
	if (it != TaskObject.end())
	{ DataType = it.value().toString();}
	else
	{
		qWarning("DataType is unknown");
		return false;
	}

	// get ImageDataFileName
	QString DataFileFullNameAndPath;
	it = TaskObject.find("ImageDataFileName");
	if (it != TaskObject.end())
	{ DataFileFullNameAndPath = Path + it.value().toString() + ".data";}
	else
	{
		qWarning("ImageDataFileName is unknown");
		return false;
	}

	// get ResultFileName
	QString ResultFileFullName;
	it = TaskObject.find("ResultFileName");
	if (it != TaskObject.end())
	{ ResultFileFullName = it.value().toString() + ".json";}
	else
	{
		qWarning("ResultFileName is unknown");
		return false;
	}

	//--------------------- Get the data ---------------------------------------//
	auto ImageData = ReadImageData(DataFileFullNameAndPath, ImageSize, DataType);
	if (ImageData == nullptr)
	{
		qWarning("ImageData is not available");
		return false;
	}
	//---------------------- Show Image ----------------------------------------//
	auto PropHandle = Figure->ShowImage(ImageData);

	//---------------------- Write Result ----------------------------------------//
	QString tempName = Path + "~temp.json";
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

	ResultFile.rename(ResultFileFullName);

	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowmesh(QString Path, QString TaskFileName)
{
	return true;
}


bool TaskHandler::run_vtkdeleteprop(QString Path, QString TaskFileName)
{

	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::RunTask(QString Path, QString TaskFileName)
{	
	// read ramdisk M:/TaskHandle/Task.json and *.data

	QFile TaskFile(Path + TaskFileName + ".json");

	if (!TaskFile.open(QIODevice::ReadOnly)) 
	{
		qWarning("Couldn't open task file (*.json)");

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
		return function(this, Path, TaskFileName);
	}
	else
	{
		return false;
	}

}

void TaskHandler::WriteExampleTaskFile(QString Path, QString FileName)
{
	QFile TaskFile(Path + FileName + ".json");

	if (!TaskFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file.");
		return;
	}

	QString Command = "vtkplotpoint";
	quint64 FigureHandle = 12345678901;
	quint64 PropHandle = 12345678902;

	QJsonObject TaskObject;

	TaskObject["Command"] = Command;

	TaskObject["FigureHandle"] = QString::number(FigureHandle);

	TaskObject["PropHandle"] = QString::number(PropHandle);

	QJsonDocument TaskDoc(TaskObject);

	TaskFile.write(TaskDoc.toJson());
}


void TaskHandler::ReadExampleTaskFile(QString Path, QString FileName)
{
	QFile TaskFile(Path + FileName + ".json");

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


vtkPoints* TaskHandler::ReadPointData(QString DataFileFullNameAndPath, quint64 PointNum, QString MatlabDataType)
{
	QFile DataFile(DataFileFullNameAndPath);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open data file.");

		return nullptr;
	}

	//---------------------------------------------------------------------------------------//
	qint64 BypesofDataFile = DataFile.size();

	qint64 BypesofPointData = 0;

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		BypesofPointData = qint64(PointNum * 8);
	}
	else if (MatlabDataType == m_MatlabDataTypeList.Single)
	{
		BypesofPointData = qint64(PointNum * 4);
	}
	else
	{
		BypesofPointData = 0;
	}

	if (BypesofDataFile != BypesofPointData)
	{
		qWarning("Data file size is not equal to point-set size");
		return nullptr;
	}
	//---------------------------------------------------------------------------------------//
	auto PointData = vtkPoints::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	PointData->SetDataType(VtkDataType);

	PointData->SetNumberOfPoints(PointNum);

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		double pos[3];

		for (quint64 i = 0; i < PointNum; ++i)
		{
			auto BypesofPos = DataFile.read((char *)pos, 24);

			if (BypesofPos != 24)
			{
				PointData->Delete();
				return nullptr;
			}

			PointData->InsertPoint(i, pos);
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
				PointData->Delete();
				return nullptr;
			}

			PointData->InsertPoint(i, pos);
		}
	}

	PointData->Modified();

	return PointData;
}


vtkPolyData* TaskHandler::ReadMeshData(QString PathAndFileName, quint64 ElementNum, QString MatlabDataType)
{
	return nullptr;
}


vtkImageData* TaskHandler::ReadImageData(QString DataFileFullNameAndPath, int ImageSize[3], QString MatlabDataType)
{
	QFile DataFile(DataFileFullNameAndPath);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning("Couldn't open data file.");

		return nullptr;
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
		return nullptr;
	}
	//---------------------------------------------------------------------------------------//

	auto ImageData = vtkImageData::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	ImageData->SetScalarType(VtkDataType, ImageData->GetInformation());

	ImageData->SetNumberOfScalarComponents(1, ImageData->GetInformation());

	qDebug() << "image data for type: " << ImageData->GetScalarTypeAsString();

	ImageData->SetDimensions(ImageSize[0], ImageSize[1], ImageSize[2]);

	int dims[3];
	ImageData->GetDimensions(dims);
	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	ImageData->AllocateScalars(ImageData->GetInformation());

	qDebug() << "image data memory size: " << ImageData->GetActualMemorySize() * 1024L;

	ImageData->GetDimensions(dims);
	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	qDebug() << "components:" << ImageData->GetNumberOfScalarComponents();
	qDebug() << "scalar size:" << ImageData->GetScalarSize();

	void *dataPtr = ImageData->GetScalarPointer();

	auto BypesofOutput = DataFile.read((char *)dataPtr, BypesofImageData);

	if (BypesofDataFile != BypesofImageData)
	{
		qWarning("Data file size is not equal to image size");
		ImageData->Delete();
		return nullptr;
	}

	ImageData->Modified();

	return ImageData;
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
}


bool TaskHandler::RemoveFolder(const QString& FolderName)
{
	bool result = true;
	QDir dir(FolderName);

	if (dir.exists(FolderName))
	{
		Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
		{
			if (info.isDir())
			{
				result = this->RemoveFolder(info.absoluteFilePath());
			}
			else
			{
				result = QFile::remove(info.absoluteFilePath());
			}

			if (!result)
			{
				return result;
			}
		}
		result = dir.rmdir(FolderName);
	}

	return result;
}