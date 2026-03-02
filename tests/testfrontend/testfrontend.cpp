#include <config.h>

#include "fileutility.h"
#include "project.h"
#include "testfrontend.h"

using namespace Tests;
using namespace Frontend;
using namespace Backend;
using namespace Backend::Core;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow;
}

//! Create the test project to view
void TestFrontend::testSetProject()
{
    // Set the files to process
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

    // Create the project
    Project project(record);
    project.responses = responses;
    project.spectrums = refSpectrums;
    project.options = solver.options;
    project.solution = solution;

    // Show the window
    mpMainWindow->setProject(project);
    mpMainWindow->show();
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
}

QTEST_MAIN(TestFrontend)
