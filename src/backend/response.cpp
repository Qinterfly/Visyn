#include "response.h"

using namespace Backend::Core;

ResponseProperties::ResponseProperties()
    : id(0)
    , direction(Direction::kNone)
    , domain(Domain::kNone)
    , dimension(Dimension::kNone)
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

VectorXd const& Response::keys() const
{
    return mKeys;
}

VectorXd const& Response::realValues() const
{
    return mRealValues;
}

VectorXcd const& Response::complexValues() const
{
    return mComplexValues;
}

double Response::key(int iSample) const
{
    if (numKeys() > 0 && iSample < mKeys.size())
        return mKeys[iSample];
    if (iSample < numValues() && props.sampleRate > std::numeric_limits<double>::epsilon())
        return 1.0 / props.sampleRate * iSample;
    return std::nan("0");
}

int Response::numKeys() const
{
    return mKeys.size();
}

int Response::numValues() const
{
    return mType == ValueType::kReal ? mRealValues.size() : mComplexValues.size();
}

bool Response::isEmpty() const
{
    return numValues() == 0;
}

void Response::setKeys(VectorXd const& keys)
{
    mKeys = keys;
}

void Response::setValues(std::vector<float> const& values)
{
    int numValues = values.size();
    VectorXd newValues(numValues);
    for (int i = 0; i != numValues; ++i)
        newValues[i] = (double) values[i];
    setValues(newValues);
}

void Response::setValues(VectorXd const& values)
{
    mType = ValueType::kReal;
    mRealValues = values;
    mComplexValues = VectorXcd();
}

void Response::setValues(VectorXcd const& values)
{
    mType = ValueType::kComplex;
    mRealValues = VectorXd();
    mComplexValues = values;
}
