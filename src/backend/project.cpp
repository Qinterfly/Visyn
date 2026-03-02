#include <matio.h>
#include <QFileInfo>

#include "project.h"

using namespace Backend::Core;

Response createResponse(QList<Segment> const& segments);

Project::Project()
{
}

Project::Project(QString const& uName)
    : name(uName)
{
}

void Project::clear()
{
    responses.clear();
    spectrums.clear();
    solution.clear();
}

bool Project::write(QString const& pathFile)
{
    bool isSuccess = true;
    QFileInfo info(pathFile);
    QString suffix = info.suffix();
    if (suffix == "mat")
    {
        mat_t* mat = Mat_CreateVer(pathFile.toStdString().c_str(), NULL, MAT_FT_DEFAULT);
        if (mat)
        {
            isSuccess &= ResponseIO::write(mat, responses, "responses");
            isSuccess &= ResponseIO::write(mat, spectrums, "spectrums");
            if (!solution.isEmpty())
            {
                isSuccess &= ResponseIO::write(mat, {solution.freqs}, "soluFreqs");
                isSuccess &= ResponseIO::write(mat, {solution.filterFreqs}, "soluFilterFreqs");
                isSuccess &= ResponseIO::write(mat, {createResponse(solution.segments)}, "soluSegmentFreqs");
                isSuccess &= ResponseIO::write(mat, {solution.spectrums}, "soluSpectrums");
            }
        }
    }
    else
    {
        qWarning() << QObject::tr("The file %1 has unknown suffix. Could not write responses").arg(pathFile);
    }
    return isSuccess;
}

//! Helper function to build up a response out of segments
Response createResponse(QList<Segment> const& segments)
{
    int numSegments = segments.size();
    VectorXd xData(numSegments);
    VectorXd yData(numSegments);
    for (int i = 0; i != numSegments; ++i)
    {
        Segment const& segment = segments[i];
        xData[i] = segment.keys.second;
        yData[i] = segment.freq;
    }
    return Response(xData, yData);
}
