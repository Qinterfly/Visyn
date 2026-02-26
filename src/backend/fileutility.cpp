#include <matio.h>
#include <thread>
#include <visom/vaufxfile.h>

#include "fileutility.h"
#include "response.h"

using namespace Backend::Core;

namespace Backend::Utility
{

// Helper function
QList<Response> readTestLabResponses(matvar_t* matVar);
QString getMatStringData(matvar_t* matVar);
MatrixXd getMatDoubleData(matvar_t* matVar);
MatrixXcd getMatComplexData(matvar_t* matVar);

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
    QList<Response> responses;

    // Check if the file exists
    QFileInfo info(pathFile);
    if (!info.exists())
    {
        qWarning() << QObject::tr("The file %1 does not exist").arg(pathFile);
        return responses;
    }

    // Read the responses
    QString suffix = info.suffix();
    if (suffix == "vaufx")
    {
        Visom::VaufxFile file(pathFile.toStdWString());
        responses = readResponses(file);
    }
    else if (suffix == "mat")
    {
        mat_t* mat = Mat_Open(pathFile.toStdString().c_str(), MAT_ACC_RDONLY);
        if (mat)
        {
            responses = readResponses(mat);
            Mat_Close(mat);
        }
    }
    else
    {
        qWarning() << QObject::tr("The file %1 has unknown suffix. Could not read responses").arg(pathFile);
    }

    return responses;
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
        props.id = 1 + i;
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

//! Read responses from a .mat file
QList<Response> readResponses(mat_t* mat)
{
    QList<Response> result;
    matvar_t* matVar = Mat_VarReadNext(mat);
    if (!matVar)
        return result;

    // Read TestLab spectrums
    if (matVar->class_type == MAT_C_STRUCT && QString(matVar->name) == "FrequencySpectrum")
        result = readTestLabResponses(matVar);

    // Clean up
    Mat_VarFree(matVar);

    return result;
}

//! Read responses from a TestLab formatted .mat file
QList<Response> readTestLabResponses(matvar_t* matVar)
{
    // Read xValues
    matvar_t* matX = Mat_VarGetStructFieldByName(matVar, "x_values", 0);
    matvar_t* matXValues = Mat_VarGetStructFieldByName(matX, "values", 0);
    MatrixXd xValues = getMatDoubleData(matXValues);
    int numResponses = xValues.cols();

    // Read yValues
    matvar_t* matY = Mat_VarGetStructFieldByName(matVar, "y_values", 0);
    matvar_t* matYValues = Mat_VarGetStructFieldByName(matY, "values", 0);
    MatrixXcd yValues = getMatComplexData(matYValues);

    // Read info
    matvar_t* matRecord = Mat_VarGetStructFieldByName(matVar, "function_record", 0);
    matvar_t* matName = Mat_VarGetStructFieldByName(matRecord, "name", 0);
    QList<QString> names(numResponses);
    if (matName->class_type == MAT_C_CELL)
    {
        matvar_t** cellsName = (matvar_t**) matName->data;
        for (int i = 0; i != numResponses; ++i)
            names[i] = getMatStringData(cellsName[i]);
    }
    else if (matName->class_type == MAT_C_CHAR)
    {
        names = {getMatStringData(matName)};
    }

    // Build up responses
    QList<Response> result(numResponses);
    for (int i = 0; i != numResponses; ++i)
    {
        Response& response = result[i];
        VectorXd keys = xValues(indexing::all, i);
        VectorXcd values = yValues(indexing::all, i);
        response.setKeys(keys);
        response.setValues(values);
        response.props.title = names[i];
        response.props.domain = Domain::kFreq;
    }

    return result;
}

//! Helper function to get a string array from a mat variable
QString getMatStringData(matvar_t* matVar)
{
    int length = matVar->nbytes / matVar->data_size;
    auto data = (char*) matVar->data;
    std::string text(data, length);
    return QString::fromStdString(text);
}

//! Helper function to get a double array from a mat variable
MatrixXd getMatDoubleData(matvar_t* matVar)
{
    // Get dimensions
    int numRows = matVar->dims[0];
    int numCols = 1;
    if (matVar->rank == 2)
        numCols = matVar->dims[1];

    // Copy the data
    auto data = static_cast<const double*>(matVar->data);
    MatrixXd result(numRows, numCols);
    int k = 0;
    for (int j = 0; j != numCols; ++j)
    {
        for (int i = 0; i != numRows; ++i)
        {
            result(i, j) = data[k];
            ++k;
        }
    }
    return result;
}

//! Helper function to get a complex array from a mat variable
MatrixXcd getMatComplexData(matvar_t* matVar)
{
    // Get dimensions
    int numRows = matVar->dims[0];
    int numCols = 1;
    if (matVar->rank == 2)
        numCols = matVar->dims[1];

    // Copy the data
    auto complexData = (const mat_complex_split_t*) matVar->data;
    auto realData = (const double*) complexData->Re;
    auto imagData = (const double*) complexData->Im;
    MatrixXcd result(numRows, numCols);
    int k = 0;
    for (int j = 0; j != numCols; ++j)
    {
        for (int i = 0; i != numRows; ++i)
        {
            result(i, j) = std::complex<double>(realData[k], imagData[k]);
            ++k;
        }
    };
    return result;
}
}
