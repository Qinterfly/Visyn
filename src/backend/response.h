#ifndef RESPONSE_H
#define RESPONSE_H

#include <complex>
#include <QList>

namespace Backend::Core
{

enum class Direction
{
    kX,
    kY,
    kZ
};

enum class Domain
{
    kTime,
    kFreq
};

enum class Dimension
{
    kAccel,
    kVel,
    kDisp,
    kSync
};

enum class ValueType
{
    kReal,
    kComplex
};

using Real = double;
using Complex = std::complex<double>;

struct ResponseProperties
{
    ResponseProperties();
    ~ResponseProperties() = default;

    int id;
    Direction direction;
    Domain domain;
    Dimension dimension;
    double sampleRate;
    QString node;
    QString component;
    QString title;
};

class Response
{
public:
    Response();
    ~Response() = default;

    ValueType type() const;
    QList<Real> const& keys() const;
    QList<Real> const& realValues() const;
    QList<Complex> const& complexValues() const;

    void setKeys(QList<Real> const& keys);
    void setValues(std::vector<float> const& values);
    void setValues(QList<Real> const& values);
    void setValues(QList<Complex> const& values);

public:
    ResponseProperties props;

private:
    ValueType mType;
    QList<Real> mKeys;
    QList<Real> mRealValues;
    QList<Complex> mComplexValues;
};

}

#endif // RESPONSE_H
