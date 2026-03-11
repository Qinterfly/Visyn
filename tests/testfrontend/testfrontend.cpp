#include <config.h>

#include "fileutility.h"
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

TestFrontend::~TestFrontend()
{
    mpMainWindow->show();
    QTest::qWait(30000);
    mpMainWindow->deleteLater();
}

QTEST_MAIN(TestFrontend)
