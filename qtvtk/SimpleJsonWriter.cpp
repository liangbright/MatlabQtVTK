#include <SimpleJsonWriter.h>
#include <QFile>
#include <QTextStream>

// Path: example: M:/xxx/, must end with /
bool SimpleJsonWriter::WritePair(std::vector<NameValuePair> PairList, QString Path, QString FileName)
{
	quint64 s = PairList.size();

	QFile JsonFile(Path + "~temp~" + FileName);

	if (!JsonFile.open(QIODevice::WriteOnly))
	{
		qWarning("Couldn't open file to save result");
		return false;
	}

	QTextStream out(&JsonFile);

	out << "{\n";
	for (quint64 i = 0; i < s; ++i)
	{
		out << "\"" << PairList[i].Name << "\"" << " : " << "\"" << PairList[i].Value << "\"";

		if (i < s - 1)
		{
			out << "," << "\n";
		}
		else
		{
			out << "\n";
		}
 	}
	out << "}\n";

	out.flush();
	if (out.status() == QTextStream::Ok)
	{
		return JsonFile.rename(Path + FileName);
	}

	return false;
}
