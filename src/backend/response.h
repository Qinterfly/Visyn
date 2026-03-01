#ifndef RESPONSE_H
#define RESPONSE_H

#include <Eigen/Core>
#include <QList>
#include <QString>

using namespace Eigen;

namespace Visom
{
class VaufxFile;
}

struct _mat_t;
typedef struct _mat_t mat_t;
struct matvar_t;

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
    double factor;
    double sampleRate;
    int numAverages;
    QString node;
    QString component;
    QString name;
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

class ResponseFile
{
public:
    ResponseFile(QString const& pathFile);
    ~ResponseFile() = default;

    QList<Response> read();
    bool write(QList<Response> const& responses);

private:
    QList<Response> read(Visom::VaufxFile& file);
    QList<Response> read(mat_t* mat);
    QList<Response> readTestLab(matvar_t* matVar);
    bool write(mat_t* mat, QList<Response> const& responses);

private:
    QString mPathFile;
};
}

#endif // RESPONSE_H
