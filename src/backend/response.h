#ifndef RESPONSE_H
#define RESPONSE_H

#include <Eigen/Core>
#include <QList>
#include <QMetaProperty>
#include <QMetaType>
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

// Helper functions
QString getLabel(Direction direction);
Direction getDirection(QString const& label);

//! Class to handle response properties
struct ResponseProperties
{
    Q_GADGET
    Q_PROPERTY(int id MEMBER id)
    Q_PROPERTY(Direction direction MEMBER direction)
    Q_PROPERTY(Domain domain MEMBER domain)
    Q_PROPERTY(Dimension dimension MEMBER dimension)
    Q_PROPERTY(int sign MEMBER sign)
    Q_PROPERTY(double sampleRate MEMBER sampleRate)
    Q_PROPERTY(QString path MEMBER path)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString node MEMBER node)
    Q_PROPERTY(QString component MEMBER component)
    Q_PROPERTY(int numAverages MEMBER numAverages)
    Q_PROPERTY(QString transducer MEMBER transducer)
    Q_PROPERTY(QString comment MEMBER comment)

public:
    ResponseProperties();
    ~ResponseProperties() = default;

    //! Channel identifier
    int id;

    //! Absolute direction
    Direction direction;

    //! Decomposition domain
    Domain domain;

    //! Measurement dimension
    Dimension dimension;

    //! Direction sign
    int sign;

    //! Recording rate
    double sampleRate;

    //! System path
    QString path;

    //! Response name
    QString name;

    //! Node name
    QString node;

    //! Component name
    QString component;

    //! Number of averages
    int numAverages;

    //! Transducer serial number
    QString transducer;

    //! User comment
    QString comment;
};

//! Class to present signal data
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
    VectorXd realSignedValues() const;
    VectorXcd complexSignedValues() const;

    VectorXd real() const;
    VectorXd imag() const;
    VectorXd amplitudes() const;
    VectorXd phases() const;

    int index(double key) const;
    double key(int iSample) const;
    double startKey() const;
    double endKey() const;
    int numKeys() const;
    int numValues() const;
    bool isEmpty() const;
    bool isComplex() const;

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

//! Class to handle serializing and deserializing responses
class ResponseIO
{
public:
    ResponseIO();
    ~ResponseIO() = default;

    static QList<Response> read(QString const& pathFile);
    static QList<Response> read(Visom::VaufxFile& file);
    static QList<Response> read(mat_t* mat);

    static bool write(QString const& pathFile, QList<Response> const& responses);
    static bool write(mat_t* mat, QList<Response> const& responses, QString const& name);

private:
    static QList<Response> readTestLabTime(matvar_t* matVar);
    static QList<Response> readTestLabFreq(matvar_t* matVar);
};
}

#endif // RESPONSE_H
