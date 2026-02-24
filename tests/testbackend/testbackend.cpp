#include <matio.h>

#include "config.h"
#include "fileutility.h"
#include "harmonicsolver.h"
#include "testbackend.h"

using namespace Tests;
using namespace Backend;
using namespace Backend::Core;

TestBackend::TestBackend()
{
}

//! Read responses from a *.vaufx file
void TestBackend::testReadVaufx()
{
    QString pathFile = Utility::combineFilePath(INPUT_DIR, "sine1.vaufx");
    QList<Response> responses = Utility::readResponses(pathFile);
    QVERIFY(responses.size() > 0);
}

//! Use harmonic solver to find segments with constant frequency
void TestBackend::testSegmentResponse()
{
    QString pathFile = Utility::combineFilePath(INPUT_DIR, "test1.mat");

    // Read the response
    auto response = readTestResponse(pathFile);
    QVERIFY(!response.isEmpty());

    // Perform the solution
    QList<Core::Response> responses = {response};
    HarmonicSolver solver(responses);
    auto solution = solver.solve();
}

//! Read a test response from a mat file
Response TestBackend::readTestResponse(QString const& pathFile)
{
    Backend::Core::Response response;

    // Helper function to retrieve data
    auto getData = [](mat_t* mat, QString const& name)
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
    };

    // Open the file for reading
    mat_t* mat = Mat_Open(pathFile.toStdString().c_str(), MAT_ACC_RDONLY);
    if (!mat)
        return response;

    // Read the keys
    response.setKeys(getData(mat, "xData"));
    response.setValues(getData(mat, "yData"));
    response.props.sampleRate = getData(mat, "sampleRate")[0];

    // Clean up
    Mat_Close(mat);

    return response;
}

QTEST_MAIN(TestBackend)
