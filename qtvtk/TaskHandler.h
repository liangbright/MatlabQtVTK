#ifndef TaskHandler_h
#define TaskHandler_h

#include <qstring.h>
#include <QStringList>
#include <qmap.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <functional>
#include <memory>
#include <unordered_map>

#include "QVtkFigure.h"

// forward declaration  -------------
class vtkDataObject;
// end of forward declaration ---------

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


class TaskHandler
{
private:

	QStringList m_MatlabCommandList;

	MatlabDataTypeList m_MatlabDataTypeList;

	QMap<QString, std::function<bool(TaskHandler*, QString, QString)>> m_MatlabCommandTranslator;

	std::unordered_map<quint64, std::unique_ptr<QVtkFigure>> m_FigureRecord;

public:
	TaskHandler();
	~TaskHandler();

	void CreateMatlabCommandList();

	void CreateMatlabCommandTranslator();

	bool RunTask(QString, QString);

	void WriteExampleTaskFile(QString, QString);
	void ReadExampleTaskFile(QString, QString);

	bool RemoveFolder(const QString&);

private:	
	//----------------------- read data file -------------------//
	vtkPoints* ReadPointData(QString, quint64, QString);

	vtkImageData* ReadImageData(QString, int[3], QString);

	vtkPolyData* ReadMeshData(QString, quint64, QString);
	//----------------------------------------------------------//

	//----------------------- process matlab command -------------------//
	bool run_vtkplotpoint(QString, QString);

	bool run_vtkshowimage(QString, QString);

	bool run_vtkshowmesh(QString, QString);
	//---------------------------------------
	bool run_vtkdeleteprop(QString, QString);

	//----------------------------------------------------------//
	
	quint64 GenerateFigureHandle();

	VtkDataTypeEnum MapMatlabDataTypeToVtkDataType(QString);

};

#endif