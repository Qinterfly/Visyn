#include <thread>
#include <visom/vaufxfile.h>

#include "fileutility.h"

using namespace Backend::Core;

namespace Backend::Utility
{

//! Open a file and check its extension
QSharedPointer<QFile> openFile(QString const& pathFile, QString const& expectedSuffix, QIODevice::OpenModeFlag const& mode)
{
    // Check if the output file has the correct extension
    QFileInfo info(pathFile);
    if (info.suffix() != expectedSuffix)
    {
        qWarning() << QObject::tr("Unknown extension was specified for the file: %1").arg(pathFile);
        return nullptr;
    }

    // Open the file for the specified mode
    QSharedPointer<QFile> pFile(new QFile(pathFile));
    if (!pFile->open(mode))
    {
        qWarning() << QObject::tr("Could not open the file: %1").arg(pathFile);
        return nullptr;
    }
    return pFile;
}

//! Read all the responses from the specified file
QList<Response> readResponses(QString const& pathFile)
{
    QList<Response> nullResult;

    // Check if the file exists
    QFileInfo info(pathFile);
    if (!info.exists())
    {
        qWarning() << QObject::tr("The file %1 does not exist").arg(pathFile);
        return nullResult;
    }

    // Read the responses
    if (info.suffix() == "vaufx")
    {
        Visom::VaufxFile file(pathFile.toStdWString());
        return readResponses(file);
    }
    else
    {
        qWarning() << QObject::tr("The file %1 has unknown suffix. Could not read responses").arg(pathFile);
    }

    return nullResult;
}

//! Read responses from a .vaufx formatted file
QList<Response> readResponses(Visom::VaufxFile& file)
{
    // Read the header
    auto header = file.readHeader();

    // Read the subheaders and assign them
    auto subheaders = file.readSubheaders(header);
    int count = header.chanCount;
    QList<Response> responses(count);
    for (int i = 0; i != count; ++i)
    {
        auto& props = responses[i].props;
        auto const& subheader = subheaders[i];
        props.id = i;
        props.domain = Domain::kTime;
        props.dimension = (Dimension) subheader.dimension;
        props.sampleRate = header.sampleRate;
        props.title = QString::fromStdWString(subheader.name);
    }

    // Helper function for reading data
    auto readData = [&file, &header, &subheaders, &responses](int i)
    {
        auto data = file.readData(header, i);
        responses[i].setValues(data);
    };

    // Start the threads
    std::vector<std::thread> threads;
    for (int i = 0; i != count; ++i)
        threads.emplace_back(readData, i);

    // Wait till the threads finish
    for (auto& t : threads)
        t.join();

    return responses;
}
}
