#include "response.h"

using namespace Backend::Core;

ResponseProperties::ResponseProperties()
    : id(0)
    , direction(Direction::kY)
    , domain(Domain::kTime)
    , dimension(Dimension::kAccel)
    , sampleRate(0.0)
{
}

Response::Response()
{
}

ValueType Response::type() const
{
    return mType;
}

QList<Real> const& Response::keys() const
{
    return mKeys;
}

QList<Real> const& Response::realValues() const
{
    return mRealValues;
}

QList<Complex> const& Response::complexValues() const
{
    return mComplexValues;
}

void Response::setKeys(QList<Real> const& keys)
{
    mKeys = keys;
}

void Response::setValues(std::vector<float> const& values)
{
    int numValues = values.size();
    QList<Real> newValues(numValues);
    for (int i = 0; i != numValues; ++i)
        newValues[i] = (Real) values[i];
    setValues(newValues);
}

void Response::setValues(QList<Real> const& values)
{
    mType = ValueType::kReal;
    mRealValues = values;
    mComplexValues = QList<Complex>();
}

void Response::setValues(QList<Complex> const& values)
{
    mType = ValueType::kComplex;
    mRealValues = QList<Real>();
    mComplexValues = values;
}
