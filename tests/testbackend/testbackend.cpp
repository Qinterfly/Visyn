#include "testbackend.h"

#include "config.h"
#include "fileutility.h"

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

QTEST_MAIN(TestBackend)
