#include <matio.h>

#include "config.h"
#include "fileutility.h"
#include "harmonicsolver.h"
#include "mathutility.h"
#include "testbackend.h"

using namespace Tests;
using namespace Backend;
using namespace Backend::Core;

// Helper function to retrieve data
VectorXd getMatData(mat_t* mat, QString const& name);

TestBackend::TestBackend()
{
}

//! Read responses from a *.vaufx file
void TestBackend::testReadVaufx()
{
    QString pathFile = Utility::combineFilePath(INPUT_DIR, "sine1.vaufx");
    QList<Response> responses = ResponseIO::read(pathFile);
    QVERIFY(responses.size() == 1);
}

void TestBackend::testReadMat()
{
    QString pathFile = Utility::combineFilePath(INPUT_DIR, "spectrums.mat");
    QList<Response> responses = ResponseIO::read(pathFile);
    QVERIFY(responses.size() == 4);
}

//! Compute statistics for a test signal
void TestBackend::testStatistics()
{
    double const precision = 1e-9;
    QString pathFile = Utility::combineFilePath(INPUT_DIR, "test1.mat");

    // Read the response
    auto response = readTestResponse(pathFile);
    QVERIFY(!response.isEmpty());

    // Compute statistics
    VectorXd const& data = response.realValues();
    double mean = Utility::mean(data);
    double median = Utility::median(data);
    double std = Utility::standardDeviation(data);
    double meanAbsDev = Utility::meanAbsoluteDeviation(data);
    double medAbsDev = Utility::medianAbsoluteDeviation(data);

    // Verify
    QVERIFY(isEqual(mean, 0.000687807407121288, precision));
    QVERIFY(isEqual(median, -0.002925881308336, precision));
    QVERIFY(isEqual(std, 0.707329064271510, precision));
    QVERIFY(isEqual(meanAbsDev, 0.635781885855727, precision));
    QVERIFY(isEqual(medAbsDev, 0.708108477389002, precision));
}

//! Use harmonic solver to find segments with constant frequency
void TestBackend::testSegmentResponse()
{
    double const precision = 1e-3;
    QString record = "test2";
    QString pathFile = Utility::combineFilePath(INPUT_DIR, record + ".mat");

    // Read the response
    auto response = readTestResponse(pathFile);
    QVERIFY(!response.isEmpty());

    // Perform the solution
    QList<Core::Response> responses = {response};
    HarmonicSolver solver(responses);
    solver.options.smoothFactor = 1.0;
    solver.options.numIter = 10;
    HarmonicSolution solution = solver.solve();

    // Set the project
    mProject = Project(record);
    mProject.responses = {response};
    mProject.solution = solution;

    // Check the solution
    QVERIFY(solution.segments.size() == 10);
    QVERIFY(isEqual(solution.segments.back().freq, 20.007633991400272, precision));
}

//! Use harmonic solver to compute harmonic responses
void TestBackend::testHarmonicResponses()
{
    QString record = "try3";
    QString pathFileResponses = Utility::combineFilePath(INPUT_DIR, record + ".vaufx");
    QString pathFileRefSpectrums = Utility::combineFilePath(INPUT_DIR, record + ".mat");

    // Read the responses
    QList<Response> responses = ResponseIO::read(pathFileResponses);

    // Read the reference spectrum
    QList<Response> refSpectrums = ResponseIO::read(pathFileRefSpectrums);

    // Perform the solution
    HarmonicSolver solver(responses);
    solver.options.smoothFactor = 1e-3;
    solver.options.numIter = 10;
    solver.options.numAverages = 1;
    solver.refSpectrums = refSpectrums;
    HarmonicSolution solution = solver.solve();
    QVERIFY(solution.segments.size() == 83);
    QVERIFY(solution.spectrums.size() == 4);

    // Write the result
    QString pathFileOutput = Utility::combineFilePath(OUTPUT_DIR, record + "-spectrums" + ".mat");
    QVERIFY(ResponseIO::write(pathFileOutput, solution.spectrums));
}

//! Write project to a file
void TestBackend::testWriteProject()
{
    QString pathFile = Utility::combineFilePath(OUTPUT_DIR, mProject.name + "-project" + ".mat");
    QVERIFY(mProject.write(pathFile));
}

//! Helper function to get data from a .mat file
VectorXd getMatData(mat_t* mat, QString const& name)
{
    VectorXd result;

    // Get the variable
    matvar_t* matVar = Mat_VarRead(mat, name.toStdString().c_str());
    if (!matVar)
        return result;

    // Get the data
    unsigned numData = matVar->nbytes / matVar->data_size;
    const double* data = static_cast<const double*>(matVar->data);

    // Copy the data
    result.resize(numData);
    std::copy_n(data, numData, result.begin());

    // Clean up
    Mat_VarFree(matVar);

    return result;
}

//! Check if two double values are equal within the specified precision
bool TestBackend::isEqual(double firstValue, double secondValue, double precision)
{
    return qAbs(firstValue - secondValue) <= precision;
}

//! Read a test response from a mat file
Response TestBackend::readTestResponse(QString const& pathFile)
{
    Backend::Core::Response response;

    // Open the file for reading
    mat_t* mat = Mat_Open(pathFile.toStdString().c_str(), MAT_ACC_RDONLY);
    if (!mat)
        return response;

    // Read the keys
    response.setKeys(getMatData(mat, "xData"));
    response.setValues(getMatData(mat, "yData"));
    response.props.sampleRate = getMatData(mat, "sampleRate")[0];

    // Clean up
    Mat_Close(mat);

    return response;
}

QTEST_MAIN(TestBackend)
