#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QDebug>
#include <QDir>
#include <QMap>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkStringArray.h>
#include <vtkStdString.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkPlane.h>
#include <vtkVolumeProperty.h>

#include <ctime>
#include <vector>

#include "TaskHandler.h"
#include "SimpleJsonWriter.h"


TaskHandler::TaskHandler()
{
	this->CreateMatlabCommandTranslator();

	this->CreateRBGColorTable();

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

    Commmand = "vtkshowaxes";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowaxes);

	Commmand = "vtkplotpoint";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplotpoint);
		
	Commmand = "vtkplotline";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplotline);
	
    Commmand = "vtkplotvector";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplotvector);

	Commmand = "vtkplottensor";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkplottensor);

	Commmand = "vtkshowpolymesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowpolymesh);

	Commmand = "vtkshowtrianglemesh";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowtrianglemesh);

	Commmand = "vtkshowvolume";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowvolume);

	Commmand = "vtkshowsliceofvolume";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkshowsliceofvolume);

	Commmand = "vtkdeleteprop";
	m_MatlabCommandList.append(Commmand);
	m_MatlabCommandTranslator[Commmand] = std::mem_fn(&TaskHandler::run_vtkdeleteprop);
	
}


void TaskHandler::CreateQVtkFigure(QVtkFigure** Figure, quint64*  FigureHandle)
{
	*Figure = nullptr;
	*FigureHandle = 0;

	auto Handle = this->GenerateFigureHandle();

	auto Figure_upt = std::unique_ptr<QVtkFigure>(new QVtkFigure);

	Figure_upt->SetHandle(Handle);

	connect(Figure_upt.get(), &QVtkFigure::UserCloseFigure, this, &TaskHandler::CloseQVtkFigure);

	QString Title = "FigureHandle = " + QString::number(Handle);

	Figure_upt->SetTitle(Title);

	Figure_upt->Show();

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
	QString tempName = TaskInfo.GetFilePath() + "~temp~" + ResultFileName;

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

	Pair.Name = "Command";
	Pair.Value = TaskInfo.Command;
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = "";
	PairList.push_back(Pair);

	Pair.Name = "FailureInfo";
	Pair.Value = FailureInfo;
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);

	return true;
}


bool TaskHandler::run_vtkfigure(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkfigure";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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
	std::vector<NameValuePair> PairList;

	NameValuePair Pair;

	Pair.Name = "IsSuccess";
	Pair.Value = "yes";
	PairList.push_back(Pair);

	Pair.Name = "FigureHandle";
	Pair.Value = QString::number(FigureHandle);
	PairList.push_back(Pair);

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowaxes(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkshowaxes";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// ----------------------- show axes -------------------------------------------//
	auto PropHandle = Figure->ShowAxes();
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run  Figure->ShowAxes";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkplotpoint(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkplotpoint";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	//get PropName if it is in the file ----------------------------------------------------------
	QString  PropName; // ""
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get Color if it is specified in the file ---------------------
	double PointColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("PointColorName");
	if (it != TaskObject.end())
	{
		auto PointColorName = it.value().toString();
		TaskHandler::GetRBGColorByName(PointColorName, PointColorValue);
	}
	else
	{
		it = TaskObject.find("PointColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				PointColorValue[0] = ColorValueList.at(0).toDouble();
				PointColorValue[1] = ColorValueList.at(1).toDouble();
				PointColorValue[2] = ColorValueList.at(2).toDouble();
			}
		}
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

	// load point data ----------------------------------------------------------------
	QString DataFilePathAndName = TaskInfo.GetFilePath() + PointDataFileName;

	qDebug() << "Read Point Data from " << DataFilePathAndName;

	vtkPolyData* PointData = nullptr;

	auto IsReadOK = this->ReadPointData(DataFilePathAndName, PointNum, PointDataType, PointData);
	if (IsReadOK == false)
	{
		qDebug() << "Can not load Point Data";

		return false;
	}

	qDebug() << "Point Data is loaded";

	// set name and color carried in FieldData of PointData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	PointData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();
	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(PointColorValue);

	PointData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();

	//---------------------- Plot Point ----------------------------------------//
	auto PropHandle = Figure->PlotPoint(PointData);
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->PlotPoint";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkplotline(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkplotline";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get LineColor  ------------------------------------------------------
	double LineColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("LineColorName");
	if (it != TaskObject.end())
	{
		auto LineColorName = it.value().toString();
		this->GetRBGColorByName(LineColorName, LineColorValue);
	}
	else
	{
		// Get Color if it is specified in the file
		bool IscolorOK = false;

		it = TaskObject.find("LineColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				LineColorValue[0] = ColorValueList.at(0).toDouble();
				LineColorValue[1] = ColorValueList.at(1).toDouble();
				LineColorValue[2] = ColorValueList.at(2).toDouble();

				IscolorOK = true;
			}
		}
	}

	// get LineNum ------------------------------------------------------
	int LineNum = 0;
	it = TaskObject.find("LineNum");
	if (it != TaskObject.end())
	{
		LineNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("LineNum is unknown");
		return false;
	}

	// get TotalPointNum ------------------------------------------------------
	int TotalPointNum = 0;
	it = TaskObject.find("TotalPointNum");
	if (it != TaskObject.end())
	{
		TotalPointNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("TotalPointNum is unknown");
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

	// load line data ----------------------------------------------------------------
	QString DataFilePathAndName = TaskInfo.GetFilePath() + PointDataFileName;
	
	vtkPolyData* LineData = nullptr;

	qDebug() << "Read Line Data from " << DataFilePathAndName;

	auto IsReadOK = ReadLineData(DataFilePathAndName, LineNum, TotalPointNum, PointDataType, LineData);

	if (IsReadOK == false)
	{
		qDebug() << "Can not load Line Data";

		return false;
	}

	qDebug() << "Line Data is loaded";

	// set name and color  in FieldData of LineData ---------------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	LineData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();

	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(LineColorValue);

	LineData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();

	//---------------------- Plot Line ----------------------------------------//
	auto PropHandle = Figure->PlotLine(LineData);
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->PlotLine";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}

bool TaskHandler::run_vtkplotvector(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkplotvector";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get Color  ------------------------------------------------------
	double VectorColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("VectorColorName");
	if (it != TaskObject.end())
	{
		auto VectorColorName = it.value().toString();
		this->GetRBGColorByName(VectorColorName, VectorColorValue);
	}
	else
	{
		// Get Color if it is specified in the file
		bool IscolorOK = false;

		it = TaskObject.find("VectorColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				VectorColorValue[0] = ColorValueList.at(0).toDouble();
				VectorColorValue[1] = ColorValueList.at(1).toDouble();
				VectorColorValue[2] = ColorValueList.at(2).toDouble();

				IscolorOK = true;
			}
		}
	}

	// get VectorNum ------------------------------------------------------
	int VectorNum = 0;
	it = TaskObject.find("VectorNum");
	if (it != TaskObject.end())
	{
		VectorNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("VectorNum is unknown");
		return false;
	}

	// get VectorDataType ------------------------------------------------------
	QString VectorDataType = 0;
	it = TaskObject.find("VectorDataType");
	if (it != TaskObject.end())
	{
		VectorDataType = it.value().toString();
	}
	else
	{
		qWarning("VectorDataType is unknown");
		return false;
	}

	// get VectorDataFileName  ------------------------------------------------------
	QString VectorDataFileName;
	it = TaskObject.find("VectorDataFileName");
	if (it != TaskObject.end())
	{
		VectorDataFileName = it.value().toString();
	}
	else
	{
		qWarning("VectorDataFileName is unknown");
		return false;
	}

	// load vector data ----------------------------------------------------------------
	QString DataFilePathAndName = TaskInfo.GetFilePath() + VectorDataFileName;

	vtkPolyData* VectorData = nullptr;

	qDebug() << "Read Vector Data from " << DataFilePathAndName;

	auto IsReadOK = ReadVectorData(DataFilePathAndName, VectorNum, VectorDataType, VectorData);

	if (IsReadOK == false)
	{
		qDebug() << "Can not load Line Data";

		return false;
	}

	qDebug() << "Vector Data is loaded";

	// set name and color carried in FieldData of VectorData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	VectorData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();

	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(VectorColorValue);

	VectorData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();

	//---------------------- Plot Vector ----------------------------------------//
	auto PropHandle = Figure->PlotVector(VectorData);
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->PlotVector";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkplottensor(const TaskInformation& TaskInfo)
{
	qDebug() << "run_vtkplottensor";

	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get Color  ------------------------------------------------------
	double TensorColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("TensorColorName");
	if (it != TaskObject.end())
	{
		auto TensorColorName = it.value().toString();
		this->GetRBGColorByName(TensorColorName, TensorColorValue);
	}
	else
	{
		// Get Color if it is specified in the file
		bool IscolorOK = false;

		it = TaskObject.find("TensorColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				TensorColorValue[0] = ColorValueList.at(0).toDouble();
				TensorColorValue[1] = ColorValueList.at(1).toDouble();
				TensorColorValue[2] = ColorValueList.at(2).toDouble();

				IscolorOK = true;
			}
		}
	}

	// get TensorNum ------------------------------------------------------
	int TensorNum = 0;
	it = TaskObject.find("TensorNum");
	if (it != TaskObject.end())
	{
		TensorNum = it.value().toString().toInt();
	}
	else
	{
		qWarning("TensorNum is unknown");
		return false;
	}

	// get TensorDataType ------------------------------------------------------
	QString TensorDataType = 0;
	it = TaskObject.find("TensorDataType");
	if (it != TaskObject.end())
	{
		TensorDataType = it.value().toString();
	}
	else
	{
		qWarning("TensorDataType is unknown");
		return false;
	}

	// get TensorDataFileName  ------------------------------------------------------
	QString TensorDataFileName;
	it = TaskObject.find("TensorDataFileName");
	if (it != TaskObject.end())
	{
		TensorDataFileName = it.value().toString();
	}
	else
	{
		qWarning("TensorDataFileName is unknown");
		return false;
	}

	// load tensor data ----------------------------------------------------------------
	QString DataFilePathAndName = TaskInfo.GetFilePath() + TensorDataFileName;

	vtkPolyData* TensorData = nullptr;

	qDebug() << "Read Tensor Data from " << DataFilePathAndName;

	auto IsReadOK = ReadTensorData(DataFilePathAndName, TensorNum, TensorDataType, TensorData);

	if (IsReadOK == false)
	{
		qDebug() << "Can not load Tensor Data";

		return false;
	}

	qDebug() << "Tensor Data is loaded";

	// set name and color carried in FieldData of TensorData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	TensorData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();

	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(TensorColorValue);

	TensorData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();

	//---------------------- Plot Tensor ----------------------------------------//
	auto PropHandle = Figure->PlotTensor(TensorData);
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->PlotTensor";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkshowpolymesh(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get MeshColor  ------------------------------------------------------
	double MeshColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("MeshColorName");
	if (it != TaskObject.end())
	{
		auto MeshColorName = it.value().toString();
		this->GetRBGColorByName(MeshColorName, MeshColorValue);
	}
	else
	{
		// Get Color if it is specified in the file
		bool IscolorOK = false;

		it = TaskObject.find("MeshColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				MeshColorValue[0] = ColorValueList.at(0).toDouble();
				MeshColorValue[1] = ColorValueList.at(1).toDouble();
				MeshColorValue[2] = ColorValueList.at(2).toDouble();

				IscolorOK = true;
			}
		}
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

	// load mesh data ----------------------------------------------------------------
	QString FilePathAndName_PointData = TaskInfo.GetFilePath() + PointDataFileName;
	QString FilePathAndName_CellData = TaskInfo.GetFilePath() + CellDataFileName;

	vtkPolyData* MeshData = nullptr;

	qDebug() << "Read Mesh Data from " << FilePathAndName_PointData << ", and " << FilePathAndName_CellData;

	auto IsReadOK = this->ReadPolyMeshData(FilePathAndName_PointData, PointNum, PointDataType,
		                                   FilePathAndName_CellData, CellNum,
										   MeshData);
	if (IsReadOK == false)
	{
		qDebug() << "Can not load Mesh Data";

		return false;
	}

	qDebug() << "Read Mesh Data is loaded";

	// set name and color carried in FieldData of MeshData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	MeshData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();

	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(MeshColorValue);

	MeshData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();
	//---------------------- Show Mesh ----------------------------------------//
	auto PropHandle = Figure->ShowPloyMesh(MeshData);
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkshowtrianglemesh(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	// get MeshColor  if it is specified in the file -------------------------
	double MeshColorValue[3] = { 1, 1, 1 };//white

	it = TaskObject.find("MeshColorName");
	if (it != TaskObject.end())
	{
		auto MeshColorName = it.value().toString();
		this->GetRBGColorByName(MeshColorName, MeshColorValue);
	}
	else
	{
		bool IscolorOK = false;

		it = TaskObject.find("MeshColorValue");
		if (it != TaskObject.end())
		{
			auto ColorValueList = it.value().toString().split(",");
			auto tempsize = ColorValueList.size();
			if (tempsize == 3)
			{
				MeshColorValue[0] = ColorValueList.at(0).toDouble();
				MeshColorValue[1] = ColorValueList.at(1).toDouble();
				MeshColorValue[2] = ColorValueList.at(2).toDouble();

				IscolorOK = true;
			}
		}
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

	// load mesh data -----------------------------------------------------------------------------
	QString FilePathAndName_PointData = TaskInfo.GetFilePath() + PointDataFileName;
	QString FilePathAndName_TriangleData = TaskInfo.GetFilePath() + TriangleDataFileName;

	vtkPolyData* MeshData = nullptr;

	qDebug() << "Read Mesh Data from " << FilePathAndName_PointData << ", and " << FilePathAndName_TriangleData;

	auto IsReadOK = this->ReadTriangleMeshData(FilePathAndName_PointData, PointNum, PointDataType,
                                    	  	   FilePathAndName_TriangleData, TriangleNum,
		                                       MeshData);
	if (IsReadOK == false)
	{
		QString FailureInfo = "Can not load Mesh Data";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	qDebug() << "Read Mesh Data is loaded";

	// set name and color carried in FieldData of MeshData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	MeshData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	auto tempColor = vtkDoubleArray::New();
	tempColor->SetNumberOfComponents(3);
	tempColor->SetName("PropColor");
	tempColor->InsertNextTuple(MeshColorValue);

	MeshData->GetFieldData()->AddArray(tempColor);
	tempColor->Delete();
	//---------------------- Show Mesh ----------------------------------------//

	auto PropHandle = Figure->ShowPloyMesh(MeshData);

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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//

	return true;
}


bool TaskHandler::run_vtkshowvolume(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	//get image size ----------------------------------------------------------
	int ImageSize[3] = { 0, 0, 0 };
	bool IsImageSizeOK = false;

	it = TaskObject.find("ImageSize");
	if (it != TaskObject.end())
	{
		auto SizeValueList = it.value().toString().split(",");
		auto tempsize = SizeValueList.size();
		if (tempsize == 3)
		{
			ImageSize[0] = int(SizeValueList.at(0).toDouble());
			ImageSize[1] = int(SizeValueList.at(1).toDouble());
			ImageSize[2] = int(SizeValueList.at(2).toDouble());

			IsImageSizeOK = true;
		}
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
	bool IsOriginOK = false;

	it = TaskObject.find("ImageOrigin");
	if (it != TaskObject.end())
	{
		auto OriginValueList = it.value().toString().split(",");
		auto tempsize = OriginValueList.size();
		if (tempsize == 3)
		{
			Origin[0] = OriginValueList.at(0).toDouble();
			Origin[1] = OriginValueList.at(1).toDouble();
			Origin[2] = OriginValueList.at(2).toDouble();

			IsOriginOK = true;
		}
	}

	if (IsOriginOK == false)
	{
		QString FailureInfo = "ImageOrigin is invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get MatlabDataType ----------------------------------------------------------
	QString DataType;
	it = TaskObject.find("DataType");
	if (it != TaskObject.end())
	{
		DataType = it.value().toString();
	}
	else
	{
		QString FailureInfo = "DataType is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get ImageDataFileName ----------------------------------------------------------
	QString DataFilePathAndName;
	it = TaskObject.find("ImageDataFileName");
	if (it != TaskObject.end())
	{
		DataFilePathAndName = TaskInfo.GetFilePath() + it.value().toString();
	}
	else
	{
		QString FailureInfo = "ImageDataFileName is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// ------------ set VolumeProperty --------------------------------------------------

	//get IntensityDisplayRange if it is in the file
	double IntensityDisplayRange[2] = { 0, 0 };
	bool IsRangeOK = false;

	it = TaskObject.find("IntensityDisplayRange");
	if (it != TaskObject.end())
	{
		auto SizeValueList = it.value().toString().split(",");
		auto tempsize = SizeValueList.size();
		if (tempsize == 2)
		{
			IntensityDisplayRange[0] = SizeValueList.at(0).toDouble();
			IntensityDisplayRange[1] = SizeValueList.at(1).toDouble();

			IsRangeOK = true;
		}
	}

	//set VolumeProperty
	vtkVolumeProperty* VolumeProperty = nullptr;

	if (IsRangeOK == true)
	{
		VolumeProperty = Figure->CreateDefaultVolumeProperty(IntensityDisplayRange);
	}

	//--------------------- Get the data ---------------------------------------//
	qDebug() << "Read Image Data from" << DataFilePathAndName;

	vtkImageData* ImageData = nullptr;
	auto IsReadOK = ReadImageData(DataFilePathAndName, ImageSize, DataType, ImageData);
	if (ImageData == nullptr)
	{
		QString FailureInfo = "ImageData is not loaded";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	qDebug() << "Image Data is loaded:" << DataFilePathAndName;

	ImageData->SetOrigin(Origin);

	// set name carried in FieldData of ImageData----------------------------
	std::string tempNameStr = PropName.toStdString();

	auto tempName = vtkStringArray::New();
	tempName->SetNumberOfComponents(3);
	tempName->SetName("PropName");
	tempName->InsertValue(0, tempNameStr);

	ImageData->GetFieldData()->AddArray(tempName);
	tempName->Delete();

	//---------------------- Show Image ----------------------------------------//
	auto PropHandle = Figure->ShowVolume(ImageData, VolumeProperty);
	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->ShowVolume";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkshowsliceofvolume(const TaskInformation& TaskInfo)
{
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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

	//get VolumeHandle ----------------------------------------------------------
	quint64 VolumeHandle = 0; // invalid handle
	it = TaskObject.find("VolumeHandle");
	if (it != TaskObject.end())
	{
		VolumeHandle = it.value().toString().toULongLong();
	}
	else
	{
		QString FailureInfo = "VolumeHandle is unknown";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	// get PropName if it is in the file -----------------------------
	QString PropName;
	it = TaskObject.find("PropName");
	if (it != TaskObject.end())
	{
		PropName = it.value().toString();
	}

	//get the SlicePlane --------------------------------
	double Origin[3] = { 0.0, 0.0, 0.0 };
	double Normal[3] = { 0.0, 0.0, 0.0 };

	bool IsPlaneOK = false;

	it = TaskObject.find("SlicePlane");
	if (it != TaskObject.end())
	{
		auto PlaneValueList = it.value().toString().split(",");
		auto tempsize = PlaneValueList.size();
		if (tempsize == 6)
		{
			Origin[0] = PlaneValueList.at(0).toDouble();
			Origin[1] = PlaneValueList.at(1).toDouble();
			Origin[2] = PlaneValueList.at(2).toDouble();

			Normal[0] = PlaneValueList.at(3).toDouble();
			Normal[1] = PlaneValueList.at(4).toDouble();
			Normal[2] = PlaneValueList.at(5).toDouble();

			IsPlaneOK = true;
		}
	}

	if (IsPlaneOK == false)
	{
		QString FailureInfo = "SlicePlane is unknown or invalid";
		TaskHandler::WriteTaskFailureInfo(TaskInfo, ResultFileName, FailureInfo);
		qWarning() << FailureInfo;
		return false;
	}

	auto SlicePlane = vtkPlane::New();
	SlicePlane->SetOrigin(Origin);
	SlicePlane->SetNormal(Normal);

	// ------------ set ImageProperty --------------------------------------------------

	//get IntensityDisplayRange if it is in the file
	double IntensityDisplayRange[2] = { 0, 0 };
	bool IsRangeOK = false;

	it = TaskObject.find("IntensityDisplayRange");
	if (it != TaskObject.end())
	{
		auto SizeValueList = it.value().toString().split(",");
		auto tempsize = SizeValueList.size();
		if (tempsize == 2)
		{
			IntensityDisplayRange[0] = SizeValueList.at(0).toDouble();
			IntensityDisplayRange[1] = SizeValueList.at(1).toDouble();

			IsRangeOK = true;
		}
	}

	//set ImageProperty
	vtkImageProperty* ImageProperty = nullptr;

	if (IsRangeOK == true)
	{
		ImageProperty = Figure->CreateDefaultImageProperty(IntensityDisplayRange);
	}

	//---------------------- show slice of the volume ---------------------//

	auto PropHandle = Figure->ShowSliceOfVolume(VolumeHandle, SlicePlane, PropName, ImageProperty);

	if (PropHandle == 0)
	{
		QString FailureInfo = "Can not run Figure->ShowSliceOfVolume";
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

	SimpleJsonWriter::WritePair(PairList, TaskInfo.GetFilePath(), ResultFileName);
	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::run_vtkdeleteprop(const TaskInformation& Task)
{

	//-----------------------------Done---------------------------------------------------//
	return true;
}


bool TaskHandler::RunTask(TaskInformation& TaskInfo)
{	
	QFile TaskFile(TaskInfo.GetFilePathAndName());

	if (!TaskFile.open(QIODevice::ReadOnly)) 
	{
		qWarning("Couldn't open task file (*.json)");
		qWarning() << "FolderPath" << TaskInfo.FolderPath;
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
	
	TaskInfo.Command = Command;

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
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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
	QFile TaskFile(TaskInfo.GetFilePathAndName());

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
// FilePathAndName, PointNum, MatlabDataType
//Output:
//  PointData: nullptr or pointer to data
bool TaskHandler::ReadPointData(QString FilePathAndName, int PointNum, QString DataType, vtkPolyData*& PointData)
{
	//---------------------------------------------------------------------------------------//
	// defalut value
	PointData = nullptr;
	//---------------------------------------------------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open data file:" << FilePathAndName;

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

	PointData = vtkPolyData::New(); 

	PointData->SetPoints(Point);
	Point->Delete();

	return true;
}


bool TaskHandler::ReadLineData(QString FilePathAndName, int LineNum, int TotalPointNum, QString DataType, vtkPolyData*& LineData)
{
	//-------------------------
	LineData = nullptr;
	//-------------------------

	if (DataType != m_MatlabDataTypeList.Double)
	{
		qWarning() << "ReadLineData only takes double DataType";
		return false;
	}

	//----------------------------------------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open line data file:" << FilePathAndName;
		return false;
	}
	//-----------------------------------
	auto Point = vtkPoints::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	Point->SetDataType(VtkDataType);
	Point->SetNumberOfPoints(TotalPointNum);

	auto LineCell = vtkCellArray::New();
	LineCell->SetNumberOfCells(LineNum);
	//-----------------------------------
	vtkIdType PointCounter = 0;
	double tempPointNum = 0;
	double tempDimension = 0;
	double tempPos[3] = { 0, 0, 0 };

	for (int i = 0; i < LineNum; ++i)
	{
		auto BypesOfPointNum = DataFile.read((char *)&tempPointNum, 8);
		if (BypesOfPointNum != 8)
		{
			Point->Delete();
			LineCell->Delete();
			return false;
		}

		tempDimension = 0;
		auto BypesOfDimension = DataFile.read((char *)&tempDimension, 8);
		if (BypesOfDimension != 8 || int(tempDimension) != 3)
		{
			Point->Delete();
			LineCell->Delete();
			return false;
		}

		LineCell->InsertNextCell(tempPointNum);

		for (int n = 0; n < tempPointNum; ++n)
		{
			auto BypesofPos = DataFile.read((char *)&tempPos, 24);
			if (BypesofPos != 24)
			{
				Point->Delete();
				LineCell->Delete();
				return false;
			}

			LineCell->InsertCellPoint(PointCounter);

			Point->InsertPoint(PointCounter, tempPos);

			PointCounter += 1;
		}
	}

	LineData = vtkPolyData::New();

	LineData->SetPoints(Point);
	Point->Delete();

	LineData->SetLines(LineCell);
	LineCell->Delete();

	return true;
}


bool TaskHandler::ReadVectorData(QString FilePathAndName, int VectorNum, QString DataType, vtkPolyData*& VectorData)
{
	//-------------------------
	VectorData = nullptr;
	//-------------------------

	if (DataType != m_MatlabDataTypeList.Double)
	{
		qWarning() << "ReadVectorData only takes double DataType";
		return false;
	}

	//----------------------------------------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open line data file:" << FilePathAndName;
		return false;
	}
	//-----------------------------------
	auto Position = vtkPoints::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	Position->SetDataType(VtkDataType);
	Position->SetNumberOfPoints(VectorNum);

	auto Vector = vtkDoubleArray::New();
	Vector->SetNumberOfComponents(3);
	Vector->SetNumberOfTuples(VectorNum);

	//-----------------------------------
	double tempVector[3] = { 0, 0, 0 };
	double tempPos[3] = { 0, 0, 0 };

	for (int i = 0; i < VectorNum; ++i)
	{
		auto BypesOfVector = DataFile.read((char *)&tempVector, 24);
		if (BypesOfVector != 24)
		{
			Position->Delete();
			Vector->Delete();
			return false;
		}

		Vector->InsertTuple(i, tempVector);
		
		auto BypesOfPosition = DataFile.read((char *)&tempPos, 24);
		if (BypesOfPosition != 24)
		{
			Position->Delete();
			Vector->Delete();
			return false;
		}

		Position->InsertPoint(i, tempPos);		
	}

    VectorData = vtkPolyData::New();

	VectorData->SetPoints(Position);
	Position->Delete();

	VectorData->GetPointData()->SetVectors(Vector);
	Vector->Delete();

	return true;
}


bool TaskHandler::ReadTensorData(QString FilePathAndName, int TensorNum, QString DataType, vtkPolyData*& TensorData)
{
	//-------------------------
	TensorData = nullptr;
	//-------------------------

	if (DataType != m_MatlabDataTypeList.Double)
	{
		qWarning() << "ReadTensorData only takes double DataType";
		return false;
	}

	//----------------------------------------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open line data file:" << FilePathAndName;
		return false;
	}
	//-----------------------------------
	auto Position = vtkPoints::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	Position->SetDataType(VtkDataType);

	Position->SetNumberOfPoints(TensorNum);

	auto Tensor = vtkDoubleArray::New();
	Tensor->SetNumberOfComponents(9);
	Tensor->SetNumberOfTuples(TensorNum);

	//-----------------------------------
	double tempTensor[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	double tempPos[3] = { 0, 0, 0 };

	long long BypesOfTensor = 0;
	long long BypesOfPosition = 0;

	for (int i = 0; i < TensorNum; ++i)
	{
		BypesOfTensor = DataFile.read((char *)&tempTensor, 72);
		if (BypesOfTensor != 72)
		{
			Position->Delete();
			Tensor->Delete();
			return false;
		}

		Tensor->InsertTuple(i, tempTensor);

		BypesOfPosition = DataFile.read((char *)&tempPos, 24);
		if (BypesOfPosition != 24)
		{
			Position->Delete();
			Tensor->Delete();
			return false;
		}

		Position->InsertPoint(i, tempPos);
	}

	TensorData = vtkPolyData::New();

	TensorData->SetPoints(Position);
	Position->Delete();

	TensorData->GetPointData()->SetTensors(Tensor);
	Tensor->Delete();

	return true;

}


bool TaskHandler::ReadPolyMeshData_From_INP(QString FilePathAndName, vtkPolyData*& MeshData)
{
	/*
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	MeshData = nullptr;
	//------------------------------ Read Point ----------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Couldn't open point data file:" << FilePathAndName;
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

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << "Couldn't open point data file:" << FilePathAndName;
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
// FilePathAndName_PointData is text file
// PointID, x, y, z
// FilePathAndName_PointData is text file
// CellID, Point1ID, Point2ID, Point3ID, ... (at least three points)
bool TaskHandler::ReadPolyMeshData(QString FilePathAndName_PointData, int PointNum, QString PointDataType,
	                               QString FilePathAndName_CellData, int CellNum,
	                               vtkPolyData*& MeshData)
{

	vtkPoints* PointData = nullptr;

	auto IsOK = this->ReadPolyMeshPointData(FilePathAndName_PointData, PointNum, PointDataType, PointData);

	if (IsOK == false)
	{
		return false;
	}

	vtkCellArray* CellData = nullptr;

	auto IsGood = this->ReadPolyMeshCellData(FilePathAndName_CellData, CellNum, CellData);

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


bool TaskHandler::ReadPolyMeshPointData(QString FilePathAndName, int PointNum, QString DataType, vtkPoints*& PointData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	PointData = nullptr;
	//------------------------------ Read Point ----------------------------------------------//
	
	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FilePathAndName;
		return false;
	}
	//-------------------------------------------------------------------
	qint64 BypesofDataFile = DataFile.size();

	qint64 BypesofPointData = 0;

	if (DataType == m_MatlabDataTypeList.Double)
	{
		BypesofPointData = qint64(PointNum * 24); // 3*8(double)
	}
	else if (DataType == m_MatlabDataTypeList.Single)
	{
		BypesofPointData = qint64(PointNum * 12); // 3*4(float)
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
	auto tempPoint = vtkPoints::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(DataType);

	tempPoint->SetDataType(VtkDataType);

	tempPoint->SetNumberOfPoints(PointNum);

	if (DataType == m_MatlabDataTypeList.Double)
	{
		double pos[3] = { 0, 0, 0 };

		for (int i = 0; i < PointNum; ++i)
		{
			auto BypesofPos = DataFile.read((char *)pos, 24);

			if (BypesofPos != 24)
			{
				tempPoint->Delete();
				return false;
			}

			tempPoint->InsertPoint(i, pos);
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
				tempPoint->Delete();
				return false;
			}

			tempPoint->InsertPoint(i, pos);
		}
	}

	PointData = tempPoint;

	return true;
}

//============================ ReadPolyMeshCellData ===================================
// datatype must be int64  (long long)
//
bool TaskHandler::ReadPolyMeshCellData(QString FilePathAndName, int CellNum, vtkCellArray*& CellData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	CellData = nullptr;

	//------------------------------ Read Cell ----------------------------------------------//
	
	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FilePathAndName;
		return false;
	}

	auto tempCell = vtkCellArray::New();

	tempCell->SetNumberOfCells(CellNum);
	//-----------------------------------

	qint64 PointNum = 0;

	qint64 PointIndex = 0;

    for (int i = 0; i < CellNum; ++i)
	{	
		auto Bypes = DataFile.read((char *)&PointNum, 8);
		if (Bypes != 8)
		{
			tempCell->Delete();
			return false;
		}

		tempCell->InsertNextCell(PointNum);

		for (int n = 0; n < PointNum; ++n)
		{
			auto BypesofRead = DataFile.read((char *)&PointIndex, 8);
			if (BypesofRead != 8)
			{
				tempCell->Delete();
				return false;
			}
			
			tempCell->InsertCellPoint(PointIndex);
		}
	}
	    
	tempCell->Squeeze();

	CellData = tempCell;

	return true;
}

//============================  ReadTriangleMeshData ==============================================
// PointData:  double or float defined by PointDataType
// TriangleData: uint64
bool TaskHandler::ReadTriangleMeshData(QString FilePathAndName_PointData, int PointNum, QString PointDataType,
	                                   QString FilePathAndName_TriangleData, int TriangleNum,
									   vtkPolyData*& MeshData)
{
	MeshData = nullptr;

	vtkPoints* PointData = nullptr;

	auto IsOK = this->ReadPolyMeshPointData(FilePathAndName_PointData, PointNum, PointDataType, PointData);

	if (IsOK == false)
	{
		return false;
	}

	vtkCellArray* TriangleData = nullptr;

	auto IsGood = this->ReadTriangleMeshTriangleData(FilePathAndName_TriangleData, TriangleNum, TriangleData);

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
bool TaskHandler::ReadTriangleMeshTriangleData(QString FilePathAndName, int TriangleNum, vtkCellArray*& TriangleData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	TriangleData = nullptr;

	//------------------------------ Read Triangle ----------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open point data file:" << FilePathAndName;
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

	auto tempTriangle = vtkCellArray::New();

	tempTriangle->SetNumberOfCells(TriangleNum);
	//-----------------------------------

	qint64 PointIndex = 0;

	for (int i = 0; i < TriangleNum; ++i)
	{
		tempTriangle->InsertNextCell(3);

		for (int n = 0; n < 3; ++n)
		{
			auto BypesofRead = DataFile.read((char *)&PointIndex, 8);
			if (BypesofRead != 8)
			{
				tempTriangle->Delete();
				return false;
			}

			tempTriangle->InsertCellPoint(PointIndex);
		}
	}

	tempTriangle->Squeeze();

	TriangleData = tempTriangle;

	return true;
}


bool TaskHandler::ReadImageData(QString FilePathAndName, const int ImageSize[3], QString MatlabDataType,
	vtkImageData*& ImageData)
{
	//---------------------------------------------------------------------------------------//
	// Initialize the output
	ImageData = nullptr;
	//---------------------------------------------------------------------------------------//

	QFile DataFile(FilePathAndName);

	if (!DataFile.open(QIODevice::ReadOnly))
	{
		qWarning() << "Couldn't open data file:" << FilePathAndName;

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
		BypesofVoxel = 4;
	}

	BypesofImageData = qint64(VoxelNum * BypesofVoxel);

	if (BypesofDataFile != BypesofImageData)
	{
		qWarning("Data file size is not equal to image size");
		return false;
	}
	//---------------------------------------------------------------------------------------//

	auto tempImage = vtkImageData::New();

	auto VtkDataType = this->MapMatlabDataTypeToVtkDataType(MatlabDataType);

	tempImage->SetDimensions(ImageSize[0], ImageSize[1], ImageSize[2]);

	tempImage->AllocateScalars(VtkDataType, 1);

	int dims[3];
	tempImage->GetDimensions(dims);

	qDebug() << "input image data dims: " << ImageSize[0] << ImageSize[1] << ImageSize[2];

	qDebug() << "image data dims: " << dims[0] << dims[1] << dims[2];

	qDebug() << "image data type: " << tempImage->GetScalarTypeAsString();

	qDebug() << "image data memory size: " << tempImage->GetActualMemorySize() * 1024L;

	qDebug() << "voxel components:" << tempImage->GetNumberOfScalarComponents();

	qDebug() << "scalar size:" << tempImage->GetScalarSize();


	//------------------------- read data----------------------------------
	// if write Data(y,x,z) in Matlab for loop: from z->y->x then

	auto BytesOfOutput = DataFile.read(static_cast<char*>(tempImage->GetScalarPointer()), BypesofImageData);

	if (BypesofImageData < 1000)
	{
		for (int z = 0; z < ImageSize[2]; ++z)
		{
			for (int y = 0; y < ImageSize[1]; ++y)
			{
				for (int x = 0; x < ImageSize[0]; ++x)
				{
					qDebug() << "xyz(" << x << "," << y << "," << z << ")=" << *(static_cast<double*>(tempImage->GetScalarPointer(x, y, z)));
				}
			}
		}
	}

	if (BytesOfOutput != BypesofImageData)
	{
		qWarning("Output Data size is not equal to image size");
		tempImage->Delete();
		return false;
	}

	ImageData = tempImage;

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

void TaskHandler::CreateRBGColorTable()
{
	RGBColor Color;

	Color.Name = "white";
	Color.Value[0] = 1;
	Color.Value[1] = 1;
	Color.Value[2] = 1;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "black";
	Color.Value[0] = 0;
	Color.Value[1] = 0;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "red";
	Color.Value[0] = 1;
	Color.Value[1] = 0;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "green";
	Color.Value[0] = 0;
	Color.Value[1] = 1;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "blue";
	Color.Value[0] = 0;
	Color.Value[1] = 0;
	Color.Value[2] = 1;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "yellow";
	Color.Value[0] = 1;
	Color.Value[1] = 1;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "cyan";
	Color.Value[0] = 0;
	Color.Value[1] = 1;
	Color.Value[2] = 1;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "magenta";
	Color.Value[0] = 1;
	Color.Value[1] = 0;
	Color.Value[2] = 1;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "silver";
	Color.Value[0] = 192 / 255.0;
	Color.Value[1] = 192 / 255.0;
	Color.Value[2] = 192 / 255.0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "orange";
	Color.Value[0] = 1;
	Color.Value[1] = 165 / 255.0;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "gold";
	Color.Value[0] = 1;
	Color.Value[1] = 215 / 255.0;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "darkred";
	Color.Value[0] = 139 / 255.0;
	Color.Value[1] = 0;
	Color.Value[2] = 0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "purple";
	Color.Value[0] = 128 / 255.0;
	Color.Value[1] = 0;
	Color.Value[2] = 128 / 255.0;
	m_RBGColorTable[Color.Name] = Color;

	Color.Name = "wheat";
	Color.Value[0] = 245 / 255.0;
	Color.Value[1] = 222 / 255.0;
	Color.Value[2] = 179 / 255.0;
	m_RBGColorTable[Color.Name] = Color;
}

bool TaskHandler::GetRBGColorByName(QString ColorName, double Value[3])
{
	auto it = m_RBGColorTable.find(ColorName);
	if (it != m_RBGColorTable.end())
	{
		auto Color = it.value();
		Value[0] = Color.Value[0];
		Value[1] = Color.Value[1];
		Value[2] = Color.Value[2];
		return true;
	}
	return false;
}