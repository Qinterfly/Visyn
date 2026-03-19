#include <config.h>

#include "fileutility.h"
#include "projecteditor.h"
#include "setupeditor.h"
#include "testfrontend.h"

using namespace Tests;
using namespace Frontend;
using namespace Backend;
using namespace Backend::Core;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow;
}

//! Create a project and resolve it
void TestFrontend::testCreateProject()
{
    // Set the files to process
    QString group = "try";
    QString record = "try3";
    QString pathFileResponses = Utility::combineFilePath(INPUT_DIR, group, record + ".vaufx");
    QString pathFileSpectrums = Utility::combineFilePath(INPUT_DIR, group, record + ".mat");

    // Retrieve the project editor
    ProjectEditor* pEditor = mpMainWindow->projectEditor();

    // Read the responses and spectrums
    pEditor->readResponses(pathFileResponses);
    pEditor->readSpectrums(pathFileSpectrums);

    // Perform the solution
    pEditor->solve();
}

//! Read a response setup from a file
void TestFrontend::testReadSetup()
{
    QString group = "try";
    QString pathFile = Utility::combineFilePath(INPUT_DIR, group, "setup.csv");

    // Retrieve the setup editor
    SetupEditor* pEditor = mpMainWindow->projectEditor()->setupEditor();

    // Read the setup
    QVERIFY(pEditor->read(pathFile));
}

//! Open a Testlab project
void TestFrontend::testOpenTestLab()
{
    QString group = "try";
    QString pathFile = Utility::combineFilePath(INPUT_DIR, group, "try.lms");

    // Retrieve the project editor
    ProjectEditor* pEditor = mpMainWindow->projectEditor();
    pEditor->openTestlab(pathFile);
}

//! Export data to a Testlab project
void TestFrontend::testExportTestlab()
{
    Project const& project = mpMainWindow->project();
    ProjectEditor* pEditor = mpMainWindow->projectEditor();
    pEditor->addResponsesTestlab(project.solution.spectrums, "Section1/try3/ResponsesSpectra");
}

TestFrontend::~TestFrontend()
{
    mpMainWindow->show();
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
}

QTEST_MAIN(TestFrontend)
