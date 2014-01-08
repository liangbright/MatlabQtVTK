#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QDebug>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>

#include <ctime>
#include <vector>

#include "TaskHandler.h"
#include "SimpleJsonWriter.h"

TaskHandler::TaskHandler()
{
	this->CreateMatlabCommandTranslator();

	m_time.start();

	m_FigureCounter = 0;
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

	Commmand = "vtkshowpolymesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowpolymesh);

	Commmand = "vtkshowtrianglemesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowtrianglemesh);

	Commmand = "vtkdeleteprop";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkdeleteprop);
	
}


void TaskHandler::CreateQVtkFigure(QVtkFigure** Figure, quint64*  FigureHandle)
{
	*Figure = nullptr;
	*FigureHandle = 0;

	auto Handle = this->GenerateFigureHandle();

	auto Figure_upt = std::unique_ptr<QVtkFigure>(new QVtkFigure(Handle));

	connect(Figure_upt.get(), &QVtkFigure::QVtkFigureClosed, this, &TaskHandler::CloseQVtkFigure);

	*FigureHandle = Handle;

	*Figure=Figure_upt.get();

	m_FigureRecord[Handle] = std::move(Figure_upt);
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


QVtkFigure* TaskHandler::GetQVtkFigure(quint64 FigureHandle)
{
	QVtkFigure* Figure = nullptr;

    auto it = m_FigureRecord.find(FigureHandle);
    if (it != m_FigureRecord.end())
    {
	    Figure = it->second.get();
	}

	return Figure;
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

	// the order of insertion is not preserved
	// output order is {FailureInfo, FigureHandle, IsSuccess, PropHandle}
	// may be a problem if sending many parameters in json file to Matlab
	// our of order -> file is not human readable
	/*
	QJsonObject ResultObject;
	ResultObject["IsSuccess"] = QString("no");

	ResultObject["FigureHandle"] = QString("");

	ResultObject["PropHandle"] = QString("");

	ResultObject["FailureInfo"] = FailureInfo;

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());
	ResultFile.close();

	ResultFile.rename(TaskInfo.Path + TaskInfo.FolderName + "/" + ResultFileName);
	*/

	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "no";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = "";
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = "";
	PairList.push_back(Pair);

	Pair.Name = "FailureInfo";
	Pair.Value = FailureInfo;
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.Path + TaskInfo.FolderName + "/", ResultFileName);

	return true;
}


bool TaskHandler::run_vtkfigure(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkfigure";

	QFile TaskFile(TaskInfo.GetFullFileNameAndPath());

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
	QVtkFigure* Figure;
	quint64  FigureHandle;
	TaskHandler::CreateQVtkFigure(&Figure, &FigureHandle);
	//---------------------- Write Result ----------------------------------------//
	/*
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

	ResultObject["PropHandle"] = QString("");

	QJsonDocument ResultDoc(ResultObject);

	ResultFile.write(ResultDoc.toJson());
	ResultFile.close();

	ResultFile.rename(TaskInfo.Path + TaskInfo.FolderName + "/" + ResultFileName);
	*/
	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = "";
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.Path + TaskInfo.FolderName + "/", ResultFileName);
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
	auto Figure = this->GetQVtkFigure(FigureHandle);
	if (Figure == nullptr)
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
	
	// Get Color if it is specified in the file
	double PointColor[3] = { 1, 1, 1 }; // {R, G, B}; white
	bool IscolorOK = true;

	it = TaskObject.find("PointColor");
	if (it != TaskObject.end())
	{
		auto ColorValueList = it.value().toString().split(",");
		auto tempsize = ColorValueList.size();
		if (tempsize == 3)
		{
			PointColor[0] = ColorValueList.at(0).toDouble();
			PointColor[1] = ColorValueList.at(1).toDouble();
			PointColor[2] = ColorValueList.at(2).toDouble();
		}
		else
		{
			IscolorOK = false;
		}
	}
	else
	{ 
		IscolorOK = false;
	}


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
	auto IsReadOK = ReadPointData(DataFileFullNameAndPath, PointNum, DataType, Point);

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
	/*
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
	*/
	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = QString::number(PropHandle);
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFullPath(), ResultFileName);
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
	auto Figure = this->GetQVtkFigure(FigureHandle);
	if (Figure == nullptr)
	{
		QString FailureInfo = "FigureHandle is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//get image size ----------------------------------------------------------
	int ImageSize[3] = { 0, 0, 0 };
	bool IsImageSizeOK = true;

	it = TaskObject.find("ImageSize");
	if (it != TaskObject.end())
	{
		auto SizeValueList = it.value().toString().split(",");
		auto tempsize = SizeValueList.size();
		if (tempsize == 3)
		{
			ImageSize[0] = SizeValueList.at(0).toInt();
			ImageSize[1] = SizeValueList.at(1).toInt();
			ImageSize[2] = SizeValueList.at(2).toInt();
		}
		else
		{
			IsImageSizeOK = false;
		}
	}
	else
	{ 
		IsImageSizeOK = false;
	}

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

	it = TaskObject.find("Origin");	
	if (it != TaskObject.end())
	{
		auto OriginValueList = it.value().toString().split(",");
		auto tempsize = OriginValueList.size();
		if (tempsize == 3)
		{
			Origin[0] = OriginValueList.at(0).toDouble();
			Origin[1] = OriginValueList.at(1).toDouble();
			Origin[2] = OriginValueList.at(2).toDouble();
		}
		else
		{
			IsOriginOK = false;
		}
	}
	else
	{ 
		IsOriginOK = false;
	}

	if (IsOriginOK == false)
	{
		qWarning() << "Origin is invalid, use [0,0,0]";
		Origin[0] = 0;
		Origin[1] = 0;
		Origin[2] = 0;
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
	QString DataFileFullName;
	it = TaskObject.find("ImageDataFileName");
	if (it != TaskObject.end())
	{
		DataFileFullName = TaskInfo.GetFullPath() + it.value().toString();
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
	bool IsRangeOK = true;
	it = TaskObject.find("DataRange");
	if (it != TaskObject.end())
	{
		auto RangeValueList = it.value().toString().split(",");
		auto tempsize = RangeValueList.size();
		if (tempsize == 2)
		{
			DataRange[0] = RangeValueList.at(0).toDouble();
			DataRange[1] = RangeValueList.at(1).toDouble();
		}
		else
		{
			IsRangeOK = false;
		}
	}
	else
	{
		IsRangeOK = false;
	}

	if (IsRangeOK == false)
	{
		QString FailureInfo = "DataRange is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	//--------------------- Get VolumeProperty ---------------------------------------
	auto VolumeProperty = Figure->GetDefaultVolumeProperty(DataRange);

	//--------------------- Get RenderMethod ---------------------------------------
	auto RenderMethod = Figure->GetDefaultRenderMethod();

	//--------------------- Get the data ---------------------------------------//
	qDebug() << "Read Image Data from" << DataFileFullName;

	vtkImageData* ImageData = nullptr;
	auto IsReadOK = ReadImageData(DataFileFullName, ImageSize, DataType, ImageData);
	if (ImageData == nullptr)
	{
		QString FailureInfo = "ImageData is not loaded";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	qDebug() << "Image Data is loaded:" << DataFileFullName;

	//---------------------- Show Image ----------------------------------------//
	auto PropHandle = Figure->ShowVolume(ImageData, VolumeProperty, RenderMethod);

	//---------------------- Write Result ----------------------------------------//
	
	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = QString::number(PropHandle);
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFullPath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowpolymesh(const TaskInformation& TaskInfo)
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

	//check FigureHandle ----------------------------------------------------------
	auto Figure = this->GetQVtkFigure(FigureHandle);
	if (Figure == nullptr)
	{
		QString FailureInfo = "FigureHandle is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get PointNum ------------------------------------------------------
	int PointNum = 0;
	it = TaskObject.find("PointNum");
	if (it != TaskObject.end())
	{
		PointNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("PointNum is unknown");
		return false;
	}
	
	// get PointDataType ------------------------------------------------------
	QString PointDataType = 0;
	it = TaskObject.find("PointDataType");
	if (it != TaskObject.end())
	{
		PointDataType = it.value().toString();
	}
	else
	{
		qWarning("PointDataType is unknown");
		return false;
	}

	// get PointDataFileName  ------------------------------------------------------
	QString PointDataFileName;
	it = TaskObject.find("PointDataFileName");
	if (it != TaskObject.end())
	{
		PointDataFileName = it.value().toString();
	}
	else
	{
		qWarning("PointDataFileName is unknown");
		return false;
	}

	// get CellNum ------------------------------------------------------
	int CellNum = 0;
	it = TaskObject.find("CellNum");
	if (it != TaskObject.end())
	{
		CellNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("CellNum is unknown");
		return false;
	}

	// get MeshColor --------------------------
	QString MeshColor = 'r';

	// get CellDataFileName  ------------------------------------------------------
	QString CellDataFileName;
	it = TaskObject.find("CellDataFileName");
	if (it != TaskObject.end())
	{
		CellDataFileName = it.value().toString();
	}
	else
	{
		qWarning("CellDataFileName is unknown");
		return false;
	}

	QString FullFileName_PointData = TaskInfo.GetFullPath() + PointDataFileName;
	QString FullFileName_CellData = TaskInfo.GetFullPath() + CellDataFileName;

	vtkPolyData* MeshData = nullptr;

	qDebug() << "Read Mesh Data from " << FullFileName_PointData << ", and " << FullFileName_CellData;

	auto IsReadOK = this->ReadPolyMeshData(FullFileName_PointData, PointNum, PointDataType, 
		                                   FullFileName_CellData, CellNum, 
										   MeshData);
	if (IsReadOK == false)
	{
		qDebug() << "Can not load Mesh Data";

		return false;
	}

	qDebug() << "Read Mesh Data is loaded";

	//---------------------- Show Mesh ----------------------------------------//

	auto PropHandle = Figure->ShowPloyMesh(MeshData, MeshColor);

	//---------------------- Write Result ----------------------------------------//

	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = QString::number(PropHandle);
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFullPath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkshowtrianglemesh(const TaskInformation& TaskInfo)
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

	//check FigureHandle ----------------------------------------------------------
	auto Figure = this->GetQVtkFigure(FigureHandle);
	if (Figure == nullptr)
	{
		QString FailureInfo = "FigureHandle is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get PointNum ------------------------------------------------------
	int PointNum = 0;
	it = TaskObject.find("PointNum");
	if (it != TaskObject.end())
	{
		PointNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("PointNum is unknown");
		return false;
	}

	// get PointDataType ------------------------------------------------------
	QString PointDataType = 0;
	it = TaskObject.find("PointDataType");
	if (it != TaskObject.end())
	{
		PointDataType = it.value().toString();
	}
	else
	{
		qWarning("PointDataType is unknown");
		return false;
	}

	// get PointDataFileName  ------------------------------------------------------
	QString PointDataFileName;
	it = TaskObject.find("PointDataFileName");
	if (it != TaskObject.end())
	{
		PointDataFileName = it.value().toString();
	}
	else
	{
		qWarning("PointDataFileName is unknown");
		return false;
	}

	// get TriangleNum ------------------------------------------------------
	int TriangleNum = 0;
	it = TaskObject.find("TriangleNum");
	if (it != TaskObject.end())
	{
		TriangleNum = it.value().toString().toInt();
	}
	else
	{
		QString FailureInfo = "TriangleNum is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get MeshColor ----------------------------------------------------------
	QString MeshColor = 'r';

	// get TriangleDataFileName  ------------------------------------------------------
	QString TriangleDataFileName;
	it = TaskObject.find("TriangleDataFileName");
	if (it != TaskObject.end())
	{
		TriangleDataFileName = it.value().toString();
	}
	else
	{
		QString FailureInfo = "TriangleDataFileName is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	QString FullFileName_PointData = TaskInfo.GetFullPath() + PointDataFileName;
	QString FullFileName_TriangleData = TaskInfo.GetFullPath() + TriangleDataFileName;

	vtkPolyData* MeshData = nullptr;

	qDebug() << "Read Mesh Data from " << FullFileName_PointData << ", and " << FullFileName_TriangleData;

	auto IsReadOK = this->ReadTriangleMeshData(FullFileName_PointData, PointNum, PointDataType,
                                    	  	   FullFileName_TriangleData, TriangleNum,
		                                       MeshData);
	if (IsReadOK == false)
	{
		QString FailureInfo = "Can not load Mesh Data";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	qDebug() << "Read Mesh Data is loaded";

	//---------------------- Show Mesh ----------------------------------------//

	auto PropHandle = Figure->ShowPloyMesh(MeshData, MeshColor);

	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run  Figure->ShowPloyMesh";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}
	//---------------------- Write Result ----------------------------------------//

	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	Pair.Name = "PropHandle";
	Pair.Value = QString::number(PropHandle);
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFullPath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

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

	qWarning() << "Unknown Matlab Command:" << Command;

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
bool TaskHandler::ReadPointData(QString DataFileFullNameAndPath, int PointNum, QString DataType, vtkPoints*& PointData)
{
	//---------------------------------------------------------------------------------------//
	// defalut value
	PointData = nullptr;
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

	if (DataType == m_MatlabDataTypeList.Double)
	{
		BypesofPointData = qint64(PointNum*3*8); // 3d point
	}
	else if (DataType == m_MatlabDataTypeList.Single)
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
	
	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	Point->SetDataType(VtkDataType);

	Point->SetNumberOfPoints(PointNum);

	if (DataType == m_MatlabDataTypeList.Double)
	{
		double pos[3];

		for (int i = 0; i < PointNum; ++i)
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
	else if (DataType == m_MatlabDataTypeList.Single)
	{
		float pos[3];

		for (int i = 0; i < PointNum; ++i)
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

	PointData = Point;

	return true;
}


bool TaskHandler::ReadImageData(QString DataFileFullNameAndPath, int ImageSize[3], QString MatlabDataType, \
	                            vtkImageData*& ImageData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	ImageData = nullptr;
	//---------------------------------------------------------------------------------------//

	QFile DataFile(DataFileFullNameAndPath);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open data file:" << DataFileFullNameAndPath;

		return false;
	}

	//---------------------------------------------------------------------------------------//
	qint64 BypesofDataFile = DataFile.size();

	auto VoxelNum = double(ImageSize[0]) * double(ImageSize[1]) * double(ImageSize[2]);

	int BypesofVoxel = 0;

	qint64 BypesofImageData = 0;

	if (MatlabDataType == m_MatlabDataTypeList.Double)
	{
		BypesofVoxel = 8;
	}
	else if (MatlabDataType == m_MatlabDataTypeList.Single)
	{
		BypesofVoxel= 4;
	}

	BypesofImageData = qint64(VoxelNum * BypesofVoxel);

	if (BypesofDataFile != BypesofImageData)
	{
		qWarning("Data file size is not equal to image size");
		return false;
	}
	//---------------------------------------------------------------------------------------//

	auto Image = vtkImageData::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	Image->SetDimensions(ImageSize[0], ImageSize[1], ImageSize[2]);

	Image->AllocateScalars(VtkDataType, 1);

	int dims[3];
	Image->GetDimensions(dims);

	qDebug() << "input image data dims: " << ImageSize[0] << ImageSize[1] << ImageSize[2];

	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	qDebug() << "image data type: " << Image->GetScalarTypeAsString();

	qDebug() << "image data memory size: " << Image->GetActualMemorySize() * 1024L;

	qDebug() << "voxel components:" << Image->GetNumberOfScalarComponents();

	qDebug() << "scalar size:" << Image->GetScalarSize();

	
	//------------------------- read data----------------------------------
	// if write Data(y,x,z) in Matlab for loop: from z->y->x then

	auto BytesOfOutput = DataFile.read(static_cast<char*>(Image->GetScalarPointer()), BypesofImageData);

	if (BypesofImageData < 1000)
	{
		for (int z = 0; z < ImageSize[2]; ++z)
		{
			for (int y = 0; y < ImageSize[1]; ++y)
			{
				for (int x = 0; x < ImageSize[0]; ++x)
				{
					qDebug() << "xyz(" <<x <<","<<y <<","<<z<<")=" <<*(static_cast<double*>(Image->GetScalarPointer(x, y, z)));
				}
			}
		}
	}

	if (BytesOfOutput != BypesofImageData)
	{
		qWarning("Output Data size is not equal to image size");
		Image->Delete();
		return false;
	}
	

	//Image->Modified();

	ImageData = Image;

	return true;
}


bool TaskHandler::ReadPolyMeshData_From_INP(QString FullFileName, vtkPolyData*& MeshData)
{
	/*
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	MeshData = nullptr;
	//------------------------------ Read Point ----------------------------------------------//

	QFile DataFile(FullFileName);

	if (!DataFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Couldn't open point data file:" << FullFileName;
		return false;
	}

	auto Point = vtkPoints::New();

	quint64 PointCounter = 0;

	Point->SetDataType(VtkDataTypeEnum::VALUE_DOUBLE);

	Point->SetNumberOfPoints(PointNum);

	QTextStream in(&DataFile);
	while (!in.atEnd())
	{
		QString line = in.readLine();  // id, x, y, z
		auto List = line.split(",");
		if (List.size() != 4)
		{
			qWarning() << "invalid Point file";
			Point->Delete();
			return false;
		}

		// weird: output 0
		//auto point_input_id = List.at(0).toInt();

		auto point_input_id = int(List.at(0).toDouble());

		//qDebug() << "point_input_id: " << point_input_id;

		PointIndexTable[point_input_id] = PointCounter;

		auto x = List.at(1).toDouble();
		auto y = List.at(2).toDouble();
		auto z = List.at(3).toDouble();

		Point->InsertPoint(PointCounter, x, y, z);

		PointCounter += 1;
	}

	PointData = Point;
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	CellData = nullptr;

	//------------------------------ Read Cell ----------------------------------------------//

	QFile DataFile(FullFileName);

	if (!DataFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Couldn't open point data file:" << FullFileName;
		return false;
	}

	auto Cell = vtkCellArray::New();

	Cell->SetNumberOfCells(CellNum);

	QTextStream in(&DataFile);
	while (!in.atEnd())
	{
		QString line = in.readLine();  // cell_id, point1_id, point2_id, point3_id, ...
		auto List = line.split(",");
		auto ListSize = List.size();
		if (ListSize < 4)
		{
			qWarning() << "invalid Cell file";
			Cell->Delete();
			return false;
		}

		auto cell_id = List.at(0).toDouble(); //useless

		Cell->InsertNextCell(ListSize - 1);

		for (int i = 1; i < ListSize; ++i)
		{
			auto point_input_id = int(List.at(i).toDouble());  //toInt() is weird: 1.00000000 ->0

			auto point_local_id = PointIndexTable.at(point_input_id);

			Cell->InsertCellPoint(point_local_id);
		}

	}

	CellData = Cell;
	*/
	return true;
}

//=================================ReadPolyMeshData=============================================
// Each cell of the mesh must be a polygon (at least a triangle, not a point or a line)
// FullFileName_PointData is text file
// PointID, x, y, z
// FullFileName_PointData is text file
// CellID, Point1ID, Point2ID, Point3ID, ... (at least three points)
bool TaskHandler::ReadPolyMeshData(QString FullFileName_PointData, int PointNum, QString PointDataType,
	                               QString FullFileName_CellData, int CellNum,
	                               vtkPolyData*& MeshData)
{

	vtkPoints* PointData = nullptr;

	auto IsOK = this->ReadPolyMeshPointData(FullFileName_PointData, PointNum, PointDataType, PointData);

	if (IsOK == false)
	{
		return false;
	}

	vtkCellArray* CellData = nullptr;

	auto IsGood = this->ReadPolyMeshCellData(FullFileName_CellData, CellNum, CellData);

	if (IsGood == false)
	{
		PointData->Delete();

		return false;
	}

	MeshData = vtkPolyData::New();

	MeshData->SetPoints(PointData);
	PointData->Delete();

	MeshData->SetPolys(CellData);
	CellData->Delete();

	return true;
}


bool TaskHandler::ReadPolyMeshPointData(QString FullFileName, int PointNum, QString DataType, vtkPoints*& PointData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	PointData = nullptr;
	//------------------------------ Read Point ----------------------------------------------//
	
	QFile DataFile(FullFileName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FullFileName;
		return false;
	}
	//-------------------------------------------------------------------
	qint64 BypesofDataFile = DataFile.size();

	qint64 BypesofPointData = 0;

	if (DataType == m_MatlabDataTypeList.Double)
	{
		BypesofPointData = qint64(PointNum * 24); // 3d point
	}
	else if (DataType == m_MatlabDataTypeList.Single)
	{
		BypesofPointData = qint64(PointNum * 12);
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

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	Point->SetDataType(VtkDataType);

	Point->SetNumberOfPoints(PointNum);

	if (DataType == m_MatlabDataTypeList.Double)
	{
		double pos[3] = { 0, 0, 0 };

		for (int i = 0; i < PointNum; ++i)
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
	else if (DataType == m_MatlabDataTypeList.Single)
	{
		float pos[3] = { 0, 0, 0 };

		for (int i = 0; i < PointNum; ++i)
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

	PointData = Point;

	return true;
}

//============================ ReadPolyMeshCellData ===================================
// datatype must be int64  (long long)
//
bool TaskHandler::ReadPolyMeshCellData(QString FullFileName, int CellNum, vtkCellArray*& CellData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	CellData = nullptr;

	//------------------------------ Read Cell ----------------------------------------------//
	
	QFile DataFile(FullFileName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FullFileName;
		return false;
	}

	auto Cell = vtkCellArray::New();

	Cell->SetNumberOfCells(CellNum);
	//-----------------------------------

	qint64 PointNum = 0;

	qint64 PointIndex = 0;

    for (int i = 0; i < CellNum; ++i)
	{	
		auto Bypes = DataFile.read((char *)&PointNum, 8);
		if (Bypes != 8)
		{
			Cell->Delete();
			return false;
		}

		Cell->InsertNextCell(PointNum);

		for (int n = 0; n < PointNum; ++n)
		{
			auto BypesofRead = DataFile.read((char *)&PointIndex, 8);
			if (BypesofRead != 8)
			{
				Cell->Delete();
				return false;
			}
			
			Cell->InsertCellPoint(PointIndex);
		}
	}
	    
	Cell->Squeeze();

	CellData = Cell;

	return true;
}

//============================  ReadTriangleMeshData ==============================================
// PointData:  double or float defined by PointDataType
// TriangleData: uint64
bool TaskHandler::ReadTriangleMeshData(QString FullFileName_PointData, int PointNum, QString PointDataType,
	                                   QString FullFileName_TriangleData, int TriangleNum,
									   vtkPolyData*& MeshData)
{
	MeshData = nullptr;

	vtkPoints* PointData = nullptr;

	auto IsOK = this->ReadPolyMeshPointData(FullFileName_PointData, PointNum, PointDataType, PointData);

	if (IsOK == false)
	{
		return false;
	}

	vtkCellArray* TriangleData = nullptr;

	auto IsGood = this->ReadTriangleMeshTriangleData(FullFileName_TriangleData, TriangleNum, TriangleData);

	if (IsGood == false)
	{
		PointData->Delete();

		return false;
	}

	MeshData = vtkPolyData::New();

	MeshData->SetPoints(PointData);
	PointData->Delete();

	MeshData->SetPolys(TriangleData);
	TriangleData->Delete();

	return true;

}


//============================ ReadTriangleMeshCellData ===================================
// datatype must be int64  (long long)
//
bool TaskHandler::ReadTriangleMeshTriangleData(QString FullFileName, int TriangleNum, vtkCellArray*& TriangleData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	TriangleData = nullptr;

	//------------------------------ Read Triangle ----------------------------------------------//

	QFile DataFile(FullFileName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FullFileName;
		return false;
	}
	//--------------------------------------------------------------
	qint64 BypesofDataFile = DataFile.size();

	if (BypesofDataFile != TriangleNum * 8 * 3)
	{
		qWarning("Data file size is not equal to input size - cell");
		return false;
	}
	//-----------------------------------------------------------

	auto Triangle = vtkCellArray::New();

	Triangle->SetNumberOfCells(TriangleNum);
	//-----------------------------------

	qint64 PointIndex = 0;

	for (int i = 0; i < TriangleNum; ++i)
	{
		Triangle->InsertNextCell(3);

		for (int n = 0; n < 3; ++n)
		{
			auto BypesofRead = DataFile.read((char *)&PointIndex, 8);
			if (BypesofRead != 8)
			{
				Triangle->Delete();
				return false;
			}

			Triangle->InsertCellPoint(PointIndex);
		}
	}

	Triangle->Squeeze();

	TriangleData = Triangle;

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
		qWarning() << "this datatype is not supported yet :" << MatlabDataType;

		return VtkDataTypeEnum::VALUE_UNKNOWN;
	}

}


quint64 TaskHandler::GenerateFigureHandle()
{
	m_FigureCounter = m_FigureCounter + 1;

	return m_FigureCounter;

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
