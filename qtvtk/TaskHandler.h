#ifndef TaskHandler_h
#define TaskHandler_h

#include <qobject.h>
#include <qstring.h>
#include <QStringList>
#include <qmap.h>
#include <QTime>

#include <vtkSmartPointer.h>
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
	QString Path;         // M:/PendingTasks/
	QString FolderName;   // abcd1234567890  is the handle of the task
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

	QString GetFullFileNameAndPath() const
	{
		return Path + FolderName + "/" + FileName;
	}

	QString GetFullPath() const
	{
		return Path + FolderName + "/";
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

public:
	TaskHandler();
	~TaskHandler();

	bool RunTask(const TaskInformation& TaskInfo);

	void WriteExampleTaskFile(const TaskInformation& TaskInfo);
	void ReadExampleTaskFile(const TaskInformation& TaskInfo);

public slots:
    void CloseQVtkFigure();

private:	

	void TaskHandler::CreateQVtkFigure(QVtkFigure** Figure, quint64*  FigureHandle);

	void CreateMatlabCommandTranslator();

	QVtkFigure* GetQVtkFigure(quint64 FigureHandle);


	//------------- Read Data From File ---------------------------------------------//

	bool ReadPointData(QString FullFileName, int PointNum, QString DataType, vtkPoints*& PointData);

	bool ReadImageData(QString FullFileName, int ImageSize[3], QString DataType, vtkImageData*& ImageData);
	
	bool ReadPolyMeshData_From_INP(QString FullFileName, vtkPolyData*& MeshData);

	bool ReadPolyMeshData(QString FullFileName_PointData, int PointNum, QString PointDataType,
		                  QString FullFileName_CellData,  int CellNum,
						  vtkPolyData*& MeshData);

	bool ReadPolyMeshPointData(QString FullFileName, int PointNum, QString PointDataType, vtkPoints*& PointData);
 
	bool ReadPolyMeshCellData(QString FullFileName,  int CellNum,  vtkCellArray*& CellData);

	bool ReadTriangleMeshData(QString FullFileName_PointData, int PointNum, QString PointDataType,
	                       	  QString FullFileName_TriangleData, int TriangleNum,
							  vtkPolyData*& MeshData);

	bool ReadTriangleMeshTriangleData(QString FullFileName, int TriangleNum, vtkCellArray*& TriangleData);

	//----------------------- process matlab command -------------------//

	bool run_vtkfigure(const TaskInformation& TaskInfo);

	//---------------------------------------

	bool run_vtkplotpoint(const TaskInformation& TaskInfo);
	//---------------------------------------

	bool run_vtkshowvolume(const TaskInformation& TaskInfo);

	bool ReadVolumeProperty(QJsonObject PropertyObject, vtkVolumeProperty* VolumeProperty);	
	//---------------------------------------

	bool run_vtkshowpolymesh(const TaskInformation& TaskInfo);
	//---------------------------------------
	
	bool run_vtkshowtrianglemesh(const TaskInformation& TaskInfo);
	//---------------------------------------

	bool run_vtkdeleteprop(const TaskInformation& TaskInfo);
	//----------------------------------------------------------//
	
	bool WriteTaskFailureInfo(const TaskInformation& TaskInfo, QString ResultFileName, QString FailureInfo);

	quint64 GenerateFigureHandle();

	VtkDataTypeEnum MapMatlabDataTypeToVtkDataType(QString MatlabDataType);

};

#endif