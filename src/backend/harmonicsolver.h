#ifndef HARMONICSOLVER_H
#define HARMONICSOLVER_H

#include "response.h"

namespace Backend::Core
{

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
    Response freqs;
    Response filterFreqs;
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
    QList<Response> const& mResponses;
};

}

#endif // HARMONICSOLVER_H
