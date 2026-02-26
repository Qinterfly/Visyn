
#ifndef FILEUTILITY_H
#define FILEUTILITY_H

#include <QDir>

namespace Visom
{
class VaufxFile;
}

struct _mat_t;
typedef struct _mat_t mat_t;

namespace Backend::Core
{
class Response;
}

namespace Backend::Utility
{

QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode);

//! Base case for combining a filepath
template<typename T>
QString combineFilePath(T const& value)
{
    return value;
}

//! Combine several components of a filepath, adding slashes if necessary
template<typename T, typename... Args>
QString combineFilePath(T const& first, Args... args)
{
    return QDir(first).filePath(combineFilePath(args...));
}

QList<Backend::Core::Response> readResponses(QString const& pathFile);
QList<Backend::Core::Response> readResponses(Visom::VaufxFile& file);
QList<Backend::Core::Response> readResponses(mat_t* mat);
}

#endif // FILEUTILITY_H
