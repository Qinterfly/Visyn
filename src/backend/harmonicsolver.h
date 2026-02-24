#ifndef HARMONICSOLVER_H
#define HARMONICSOLVER_H

#include <Eigen/Core>
#include <QList>

namespace Backend::Core
{

class Response;

struct HarmonicOptions
{
    HarmonicOptions();
    ~HarmonicOptions() = default;

    int iSync;
    int numIter;
    double smoothFactor;
};

struct HarmonicSolution
{
};

//! Class to convert time responses to harmonic ones
class HarmonicSolver
{
public:
    HarmonicSolver(QList<Response> const& responses);
    ~HarmonicSolver() = default;

    HarmonicSolution solve();

public:
    HarmonicOptions options;

private:
    void evaluateFreqs(Response const& response);

private:
    QList<Response> const& mResponses;
    Eigen::VectorXd mXFreqs;
    Eigen::VectorXd mYFreqs;
};

}

#endif // HARMONICSOLVER_H
