
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
    void testSegmentResponse();

private:
    Backend::Core::Response readTestResponse(QString const& pathFile);
};
}

#endif // TESTBACKEND_H
