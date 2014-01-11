#ifndef TaskHandler_h
#define TaskHandler_h

#include <qobject.h>
#include <qstring.h>
#include <QStringList>
#include <qmap.h>
#include <QTime>

#include <vtkType.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <map>

#include "QVtkFigure.h"

class vtkVolumeProperty;

class TaskInformation
{
public:
	// M:/PendingTasks/abcd1234567890/Task.json
	QString FolderPath;   // M:/PendingTasks/

	QString FolderName;   // abcd1234567890  is the handle of the task

	QString Command;

private:
	QString FileName;     // Task.json   the name is fixed for every task

public:
	TaskInformation()
	{
		FileName = "Task.json";
	}

	~TaskInformation()
	{
	}

	QString GetFileName() const
	{
		return  FileName;
	}

	QString GetFilePathAndName() const
	{
		return FolderPath + FolderName + "/" + FileName;
	}

	QString GetFilePath() const
	{
		return FolderPath + FolderName + "/";
	}
};

class MatlabDataTypeList
{
public:
	QString Double = "double";
	QString Single = "single";

	QString Int8 = "int8";
	QString Int16 = "int16";
	QString Int32 = "int32";
	QString Int64 = "int64";

	QString UInt8 = "uint8";
	QString UInt16 = "uint16";
	QString UInt32 = "uint32";
	QString UInt64 = "uint64";

	QString Unknown = "unknown";
};

typedef enum
{
	VALUE_VOID = VTK_VOID,
	VALUE_BIT = VTK_BIT,
	VALUE_CHAR = VTK_CHAR,
	VALUE_SIGNED_CHAR = VTK_SIGNED_CHAR,
	VALUE_UNSIGNED_CHAR = VTK_UNSIGNED_CHAR,
	VALUE_SHORT = VTK_SHORT,
	VALUE_UNSIGNED_SHORT = VTK_UNSIGNED_SHORT,
	VALUE_INTEGER = VTK_INT,
	VALUE_UNSIGNED_INTEGER = VTK_UNSIGNED_INT,
	VALUE_LONG = VTK_LONG,
	VALUE_UNSIGNED_LONG = VTK_UNSIGNED_LONG,
	VALUE_FLOAT = VTK_FLOAT,
	VALUE_DOUBLE = VTK_DOUBLE,
	VALUE_IDTYPE = VTK_ID_TYPE,
	VALUE_STRING = VTK_STRING,

	VALUE_UNKNOWN

} VtkDataTypeEnum;


class RGBColor
{
public:
	QString Name;
	double Value[3];
public:
	RGBColor()
	{
		Name = "white";
		Value[0] = 1;
		Value[1] = 1;
		Value[2] = 1;
	}
};

class TaskHandler : public QObject
{
	Q_OBJECT;

private:

	QStringList m_MatlabCommandList;

	MatlabDataTypeList m_MatlabDataTypeList;

	QMap<QString, std::function<bool(TaskHandler*, const TaskInformation&)>> m_MatlabCommandTranslator;

	// <FigureHandle, UniquePtr_Figure>
	std::unordered_map<quint64, std::unique_ptr<QVtkFigure>> m_FigureRecord; 

	QTime m_time;

	quint64 m_FigureCounter;

	QMap<QString, RGBColor> m_RBGColorTable;

public:
	TaskHandler();
	~TaskHandler();

	bool RunTask(TaskInformation& TaskInfo);

	void WriteExampleTaskFile(const TaskInformation& TaskInfo);
	void ReadExampleTaskFile(const TaskInformation& TaskInfo);

public slots:
    void CloseQVtkFigure();

private:	

	void CreateQVtkFigure(QVtkFigure** Figure, quint64*  FigureHandle);

	void CreateMatlabCommandTranslator();

	QVtkFigure* GetQVtkFigure(quint64 FigureHandle);

	void CreateRBGColorTable();
	bool GetRBGColorByName(QString ColorName, double* Value);

	//------------- Read Data From File ---------------------------------------------//

	bool ReadPointData(QString FilePathAndName, int PointNum, QString DataType, vtkPolyData*& PointData);

	bool ReadLineData(QString FilePathAndName, int LineNum, int TotalPointNum, QString DataType, vtkPolyData*& LineData);
	
	bool ReadVectorData(QString FilePathAndName, int VectorNum, QString VectorDataType, vtkPolyData*& VectorData);

	bool ReadTensorData(QString FilePathAndName, int TensorNum, QString TensorDataType, vtkPolyData*& TensorData);

	bool ReadPolyMeshData_From_INP(QString FilePathAndName, vtkPolyData*& MeshData);

	bool ReadPolyMeshData(QString FilePathAndName_PointData, int PointNum, QString PointDataType,
		                  QString FilePathAndName_CellData,  int CellNum,
						  vtkPolyData*& MeshData);

	bool ReadPolyMeshPointData(QString FilePathAndName, int PointNum, QString PointDataType, vtkPoints*& PointData);
 
	bool ReadPolyMeshCellData(QString FilePathAndName,  int CellNum,  vtkCellArray*& CellData);

	bool ReadTriangleMeshData(QString FilePathAndName_PointData, int PointNum, QString PointDataType,
	                       	  QString FilePathAndName_TriangleData, int TriangleNum,
							  vtkPolyData*& MeshData);

	bool ReadTriangleMeshTriangleData(QString FilePathAndName, int TriangleNum, vtkCellArray*& TriangleData);

	bool ReadImageData(QString FilePathAndName, const int ImageSize[3], QString DataType, vtkImageData*& ImageData);

	//----------------------- process matlab command -------------------//

	bool run_vtkfigure(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkshowaxes(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkplotpoint(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkplotline(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkplotvector(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkplottensor(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkshowpolymesh(const TaskInformation& TaskInfo);
	//----------------------------------------------------------
	
	bool run_vtkshowtrianglemesh(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkshowvolume(const TaskInformation& TaskInfo);

	bool ReadVolumeProperty(const QJsonObject& PropertyObject, vtkVolumeProperty*& VolumeProperty);
	//----------------------------------------------------------

	bool run_vtkshowsliceofvolume(const TaskInformation& TaskInfo);
	//----------------------------------------------------------

	bool run_vtkremoveprop(const TaskInformation& TaskInfo);
	//----------------------------------------------------------//
	
	bool WriteTaskFailureInfo(const TaskInformation& TaskInfo, QString ResultFileName, QString FailureInfo);

	quint64 GenerateFigureHandle();

	VtkDataTypeEnum MapMatlabDataTypeToVtkDataType(QString MatlabDataType);

};

#endif