#ifndef SimpleJsonWriter_h
#define SimpleJsonWriter_h

#include <QString>
#include <unordered_map>

class NameValuePair
{
public:
	QString  Name;
	QString  Value;
};

class SimpleJsonWriter
{
public:
	static bool WritePair(std::vector<NameValuePair>, QString FilePath, QString FileName);

};

#endif