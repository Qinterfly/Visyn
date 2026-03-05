#include <matio.h>
#include <thread>
#include <visom/vaufxfile.h>
#include <QFileInfo>

#include "response.h"

using namespace Backend::Core;

// Helper function
QString getMatStringData(matvar_t* matVar);
QStringList getMatStringsData(matvar_t* matVar);
MatrixXd getMatDoubleData(matvar_t* matVar);
MatrixXcd getMatComplexData(matvar_t* matVar);
matvar_t* createVariable(double value);
matvar_t* createVariable(QString const& value);
matvar_t* createVariable(VectorXd const& data);
matvar_t* createVariable(VectorXcd const& data);

ResponseProperties::ResponseProperties()
    : id(0)
    , direction(Direction::kNone)
    , domain(Domain::kNone)
    , dimension(Dimension::kNone)
    , sampleRate(0.0)
    , numAverages(0)
{
}

Response::Response()
{
}

Response::Response(VectorXd const& values)
{
    setValues(values);
}

Response::Response(VectorXcd const& values)
{
    setValues(values);
}

Response::Response(VectorXd const& keys, VectorXd const& values)
{
    setKeys(keys);
    setValues(values);
}

Response::Response(VectorXd const& keys, VectorXcd const& values)
{
    setKeys(keys);
    setValues(values);
}

ValueType Response::type() const
{
    return mType;
}

VectorXd const& Response::keys() const
{
    return mKeys;
}

VectorXd const& Response::realValues() const
{
    return mRealValues;
}

VectorXcd const& Response::complexValues() const
{
    return mComplexValues;
}

VectorXd Response::real() const
{
    if (isComplex())
        return mComplexValues.real();
    else
        return mRealValues;
}

VectorXd Response::imag() const
{
    if (isComplex())
        return mComplexValues.imag();
    else
        return {};
}

VectorXd Response::amplitudes() const
{
    if (isComplex())
        return mComplexValues.cwiseAbs();
    else
        return mRealValues.cwiseAbs();
}

VectorXd Response::phases() const
{
    if (mType != ValueType::kComplex)
        return {};
    int count = numValues();
    VectorXd result(count);
    for (int i = 0; i != count; ++i)
    {
        auto value = mComplexValues[i];
        result[i] = std::atan2(value.imag(), value.real());
    }
    return result;
}

int Response::index(double key) const
{
    int iFound = -1;
    int count = numKeys();
    if (count > 0)
    {
        double minDist = std::numeric_limits<double>::max();
        for (int i = 0; i != count; ++i)
        {
            double dist = std::abs(mKeys[i] - key);
            if (dist < minDist)
            {
                iFound = i;
                minDist = dist;
            }
        }
    }
    else
    {
        iFound = floor(key * props.sampleRate);
    }
    return iFound;
}

double Response::key(int iSample) const
{
    if (numKeys() > 0 && iSample < mKeys.size())
        return mKeys[iSample];
    if (iSample < numValues() && props.sampleRate > std::numeric_limits<double>::epsilon())
        return 1.0 / props.sampleRate * iSample;
    return std::nan("0");
}

int Response::numKeys() const
{
    return mKeys.size();
}

int Response::numValues() const
{
    return mType == ValueType::kReal ? mRealValues.size() : mComplexValues.size();
}

bool Response::isEmpty() const
{
    return numValues() == 0;
}

bool Response::isComplex() const
{
    return mType == ValueType::kComplex;
}

void Response::setKeys(VectorXd const& keys)
{
    mKeys = keys;
}

void Response::setValues(std::vector<float> const& values)
{
    int numValues = values.size();
    VectorXd newValues(numValues);
    for (int i = 0; i != numValues; ++i)
        newValues[i] = (double) values[i];
    setValues(newValues);
}

void Response::setValues(VectorXd const& values)
{
    mType = ValueType::kReal;
    mRealValues = values;
    mComplexValues = VectorXcd();
}

void Response::setValues(VectorXcd const& values)
{
    mType = ValueType::kComplex;
    mRealValues = VectorXd();
    mComplexValues = values;
}

ResponseIO::ResponseIO()
{
}

//! Read all the responses from the specified file
QList<Response> ResponseIO::read(QString const& pathFile)
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
        responses = read(file);
    }
    else if (suffix == "mat")
    {
        mat_t* mat = Mat_Open(pathFile.toStdString().c_str(), MAT_ACC_RDONLY);
        if (mat)
        {
            responses = read(mat);
            Mat_Close(mat);
        }
    }
    else
    {
        qWarning() << QObject::tr("The file %1 has unknown suffix. Could not read responses").arg(pathFile);
    }

    return responses;
}

//! Write all responses to the specified file
bool ResponseIO::write(QString const& pathFile, QList<Response> const& responses)
{
    bool isSuccess = false;
    QFileInfo info(pathFile);
    QString suffix = info.suffix();
    if (suffix == "mat")
    {
        mat_t* mat = Mat_CreateVer(pathFile.toStdString().c_str(), NULL, MAT_FT_DEFAULT);
        if (mat)
        {
            isSuccess = write(mat, responses, "responses");
            Mat_Close(mat);
        }
    }
    else
    {
        qWarning() << QObject::tr("The file %1 has unknown suffix. Could not write responses").arg(pathFile);
    }
    return isSuccess;
}

//! Read responses from a .vaufx formatted file
QList<Response> ResponseIO::read(Visom::VaufxFile& file)
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
        props.name = QString::fromStdU16String(subheader.name);
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
QList<Response> ResponseIO::read(mat_t* mat)
{
    QList<Response> result;

    // Process all variables
    while (matvar_t* matVar = Mat_VarReadNext(mat))
    {
        if (!matVar)
            return result;

        // Read TestLab files
        QString name = matVar->name;
        if (matVar->class_type == MAT_C_STRUCT)
        {
            QList<Response> responses;
            if (name.contains("Signal"))
                responses = readTestLabTime(matVar);
            else
                responses = readTestLabFreq(matVar);
            if (!responses.isEmpty())
                result.append(std::move(responses));
        }

        // Clean up
        Mat_VarFree(matVar);
    }

    return result;
}

//! Write responses to a .mat file
bool ResponseIO::write(mat_t* mat, QList<Response> const& responses, QString const& name)
{
    // Constants
    const char* kFieldNames[] = {"keys", "values", "name", "sampleRate"};
    int const kNumFields = 4;

    // Create the array of structures
    size_t numResponses = responses.size();
    size_t dims[2] = {numResponses, 1};
    matvar_t* matResponses = Mat_VarCreateStruct(name.toStdString().c_str(), 2, dims, kFieldNames, kNumFields);

    // Add all the responses
    for (size_t iResponse = 0; iResponse != numResponses; ++iResponse)
    {
        Response const& response = responses[iResponse];

        // Create keys field
        matvar_t* matKeys = createVariable(response.keys());
        Mat_VarSetStructFieldByName(matResponses, kFieldNames[0], iResponse, matKeys);

        // Create values field
        matvar_t* matValues = response.isComplex() ? createVariable(response.complexValues()) : createVariable(response.realValues());
        Mat_VarSetStructFieldByName(matResponses, kFieldNames[1], iResponse, matValues);

        // Create properties
        Mat_VarSetStructFieldByName(matResponses, kFieldNames[2], iResponse, createVariable(response.props.name));
        Mat_VarSetStructFieldByName(matResponses, kFieldNames[3], iResponse, createVariable(response.props.sampleRate));
    }

    // Write struct array to file
    Mat_VarWrite(mat, matResponses, MAT_COMPRESSION_ZLIB);

    // Clean up
    Mat_VarFree(matResponses);

    return true;
}

//! Read spectrums from a TestLab formatted .mat file
QList<Response> ResponseIO::readTestLabTime(matvar_t* matVar)
{
    QList<Response> nullResult;

    // Read xValues
    matvar_t* matX = Mat_VarGetStructFieldByName(matVar, "x_values", 0);
    if (!matX)
        return nullResult;
    matvar_t* matXStartValue = Mat_VarGetStructFieldByName(matX, "start_value", 0);
    matvar_t* matXIncrement = Mat_VarGetStructFieldByName(matX, "increment", 0);
    matvar_t* matXNumValues = Mat_VarGetStructFieldByName(matX, "number_of_values", 0);
    double xStart = getMatDoubleData(matXStartValue)(0);
    double xStep = getMatDoubleData(matXIncrement)(0);
    int numValues = getMatDoubleData(matXNumValues)(0);
    VectorXd xValues = VectorXd::LinSpaced(numValues, xStart, xStart + (numValues - 1) * xStep);
    double sampleRate = xStep > std::numeric_limits<double>::epsilon() ? 1.0 / xStep : 0.0;

    // Read yValues
    matvar_t* matY = Mat_VarGetStructFieldByName(matVar, "y_values", 0);
    if (!matY)
        return nullResult;
    matvar_t* matYValues = Mat_VarGetStructFieldByName(matY, "values", 0);
    MatrixXd yValues = getMatDoubleData(matYValues);
    int numResponses = yValues.cols();

    // Read info
    matvar_t* matRecord = Mat_VarGetStructFieldByName(matVar, "function_record", 0);
    matvar_t* matName = Mat_VarGetStructFieldByName(matRecord, "name", 0);
    QList<QString> names = getMatStringsData(matName);

    // Build up responses
    QList<Response> result(numResponses);
    for (int i = 0; i != numResponses; ++i)
    {
        Response& response = result[i];
        VectorXd const& keys = xValues;
        VectorXd values = yValues(indexing::all, i);
        response.setKeys(keys);
        response.setValues(values);
        response.props.name = names[i];
        response.props.domain = Domain::kTime;
        response.props.sampleRate = sampleRate;
    }

    return result;
}

//! Read time responses from a TestLab formatted .mat file
QList<Response> ResponseIO::readTestLabFreq(matvar_t* matVar)
{
    QList<Response> nullResult;

    // Read xValues
    matvar_t* matX = Mat_VarGetStructFieldByName(matVar, "x_values", 0);
    if (!matX)
        return nullResult;
    matvar_t* matXValues = Mat_VarGetStructFieldByName(matX, "values", 0);
    MatrixXd xValues = getMatDoubleData(matXValues);

    // Read yValues
    matvar_t* matY = Mat_VarGetStructFieldByName(matVar, "y_values", 0);
    if (!matY)
        return nullResult;
    matvar_t* matYValues = Mat_VarGetStructFieldByName(matY, "values", 0);
    MatrixXcd yValues = getMatComplexData(matYValues);

    // Read info
    matvar_t* matRecord = Mat_VarGetStructFieldByName(matVar, "function_record", 0);
    matvar_t* matName = Mat_VarGetStructFieldByName(matRecord, "name", 0);
    QList<QString> names = getMatStringsData(matName);

    // Build up responses
    int numResponses = yValues.cols();
    QList<Response> result(numResponses);
    for (int i = 0; i != numResponses; ++i)
    {
        Response& response = result[i];
        VectorXd keys = xValues(indexing::all, i);
        VectorXcd values = yValues(indexing::all, i);
        response.setKeys(keys);
        response.setValues(values);
        response.props.name = names[i];
        response.props.domain = Domain::kFreq;
    }

    return result;
}

//! Helper function to get a string from a mat variable
QString getMatStringData(matvar_t* matVar)
{
    size_t length = matVar->nbytes / matVar->data_size;
    auto data = (char*) matVar->data;
    std::string text(data, length);
    return QString::fromStdString(text);
}

//! Helper function to get a string array from a mat variable
QStringList getMatStringsData(matvar_t* matVar)
{
    QList<QString> result;
    if (matVar->class_type == MAT_C_CELL)
    {
        // Count the number of strings to read
        size_t rank = matVar->rank;
        size_t numResult = 1;
        for (size_t i = 0; i != rank; ++i)
            numResult *= matVar->dims[i];
        result.resize(numResult);

        // Read all the strings
        matvar_t** cellsName = (matvar_t**) matVar->data;
        for (int i = 0; i != numResult; ++i)
            result[i] = getMatStringData(cellsName[i]);
    }
    else if (matVar->class_type == MAT_C_CHAR)
    {
        result = {getMatStringData(matVar)};
    }
    return result;
}

//! Helper function to get a double array from a mat variable
MatrixXd getMatDoubleData(matvar_t* matVar)
{
    // Get dimensions
    size_t numRows = matVar->dims[0];
    size_t numCols = 1;
    if (matVar->rank == 2)
        numCols = matVar->dims[1];

    // Copy the data
    auto data = static_cast<const double*>(matVar->data);
    MatrixXd result(numRows, numCols);
    size_t k = 0;
    for (size_t j = 0; j != numCols; ++j)
    {
        for (size_t i = 0; i != numRows; ++i)
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
    size_t numRows = matVar->dims[0];
    size_t numCols = 1;
    if (matVar->rank == 2)
        numCols = matVar->dims[1];

    // Copy the data
    auto complexData = (const mat_complex_split_t*) matVar->data;
    auto realData = (const double*) complexData->Re;
    auto imagData = (const double*) complexData->Im;
    MatrixXcd result(numRows, numCols);
    size_t k = 0;
    for (size_t j = 0; j != numCols; ++j)
    {
        for (size_t i = 0; i != numRows; ++i)
        {
            result(i, j) = std::complex<double>(realData[k], imagData[k]);
            ++k;
        }
    };
    return result;
}

//! Helper function to create a variable related to a double value
matvar_t* createVariable(double value)
{
    size_t const rank = 1;
    size_t dims[rank] = {1};
    return Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 1, dims, &value, 0);
}

//! Helper function to create a variable related to a string value
matvar_t* createVariable(QString const& value)
{
    size_t const rank = 1;
    QByteArray array = value.toUtf8();
    std::string text(array.data(), array.length());
    size_t dims[rank] = {(size_t) text.length()};
    return Mat_VarCreate(NULL, MAT_C_CHAR, MAT_T_UTF8, rank, dims, text.data(), 0);
}

//! Helper function to create a variable related to a double array
matvar_t* createVariable(VectorXd const& data)
{
    size_t const rank = 2;
    size_t dims[rank] = {(size_t) data.rows(), (size_t) data.cols()};
    return Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, rank, dims, data.data(), 0);
}

//! Helper function to create a variable related to a complex array
matvar_t* createVariable(VectorXcd const& data)
{
    size_t const rank = 2;
    size_t dims[rank] = {(size_t) data.rows(), (size_t) data.cols()};
    size_t N = data.size();
    double* real = (double*) malloc(N * sizeof(double));
    double* imag = (double*) malloc(N * sizeof(double));
    for (size_t i = 0; i != N; ++i)
    {
        std::complex<double> const& value = data[i];
        real[i] = value.real();
        imag[i] = value.imag();
    }
    mat_complex_split_t complexData = {real, imag};
    return Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, rank, dims, &complexData, MAT_F_COMPLEX);
}
