
#ifndef TESTBACKEND_H
#define TESTBACKEND_H

#include <QTest>

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
    // File
    void testReadVaufx();

    // Math
    void testStatistics();
    // void testSegmentResponse();
    void testHarmonicResponses();

private:
    bool isEqual(double firstValue, double secondValue, double precision);
    Backend::Core::Response readTestResponse(QString const& pathFile);
};
}

#endif // TESTBACKEND_H
