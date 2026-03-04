#include <config.h>

#include "fileutility.h"
#include "project.h"
#include "projecteditor.h"
#include "testfrontend.h"

using namespace Tests;
using namespace Frontend;
using namespace Backend;
using namespace Backend::Core;

TestFrontend::TestFrontend()
{
    mpMainWindow = new MainWindow;
}

//! Create a project
void TestFrontend::testProjectEditor()
{
    // Set the files to process
    QString record = "try3";
    QString pathFileResponses = Utility::combineFilePath(INPUT_DIR, record + ".vaufx");
    QString pathFileSpectrums = Utility::combineFilePath(INPUT_DIR, record + ".mat");

    // Retrieve the project editor
    ProjectEditor* pEditor = mpMainWindow->projectEditor();

    // Read the responses and spectrums
    pEditor->readResponses(pathFileResponses);
    pEditor->readSpectrums(pathFileSpectrums);

    // Perform the solution
    pEditor->solve();

    // Show the window
    mpMainWindow->show();
}

TestFrontend::~TestFrontend()
{
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
}

QTEST_MAIN(TestFrontend)
