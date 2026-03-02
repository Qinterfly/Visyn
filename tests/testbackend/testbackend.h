
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

#include "project.h"
#include "response.h"

namespace Tests
{

class TestBackend : public QObject
{
    Q_OBJECT

public:
    TestBackend();
    virtual ~TestBackend() = default;

private slots:
    // Read
    void testReadVaufx();
    void testReadMat();

    // Math
    void testStatistics();
    void testSegmentResponse();
    void testHarmonicResponses();

    // Project
    void testWriteProject();

private:
    bool isEqual(double firstValue, double secondValue, double precision);
    Backend::Core::Response readTestResponse(QString const& pathFile);

private:
    Backend::Core::Project mProject;
};
}

#endif // TESTBACKEND_H
