#ifndef RESPONSE_H
#define RESPONSE_H

#include <Eigen/Core>
#include <QString>

using namespace Eigen;

namespace Backend::Core
{

enum class Direction
{
    kNone,
    kX,
    kY,
    kZ
};

enum class Domain
{
    kNone,
    kTime,
    kFreq
};

enum class Dimension
{
    kNone,
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

struct ResponseProperties
{
    ResponseProperties();
    ~ResponseProperties() = default;

    int id;
    Direction direction;
    Domain domain;
    Dimension dimension;
    double sampleRate;
    int numAverages;
    QString node;
    QString component;
    QString title;
    QString info;
};

class Response
{
public:
    Response();
    Response(VectorXd const& values);
    Response(VectorXcd const& values);
    Response(VectorXd const& keys, VectorXd const& values);
    Response(VectorXd const& keys, VectorXcd const& values);
    ~Response() = default;

    ValueType type() const;
    VectorXd const& keys() const;
    VectorXd const& realValues() const;
    VectorXcd const& complexValues() const;
    VectorXd phases() const;

    int index(double key) const;
    double key(int iSample) const;
    int numKeys() const;
    int numValues() const;
    bool isEmpty() const;

    void setKeys(VectorXd const& keys);
    void setValues(std::vector<float> const& values);
    void setValues(VectorXd const& values);
    void setValues(VectorXcd const& values);

public:
    ResponseProperties props;

private:
    ValueType mType;
    VectorXd mKeys;
    VectorXd mRealValues;
    VectorXcd mComplexValues;
};

}

#endif // RESPONSE_H
