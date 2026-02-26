#ifndef HARMONICSOLVER_H
#define HARMONICSOLVER_H

#include <QList>

#include "response.h"

namespace Backend::Core
{

using PairDouble = QPair<double, double>;

struct Segment
{
    QPair<int, int> indices;
    QPair<double, double> keys;
    double freq;
};

struct HarmonicOptions
{
    HarmonicOptions();
    ~HarmonicOptions() = default;

    int iSync;
    int numIter;
    double smoothFactor;
    int numSkipPeriods;
    int numAverages;
};

struct HarmonicSolution
{
    Response freqs;
    Response filterFreqs;
    QList<Segment> segments;
    QList<Response> spectrums;
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
    Response refSpectrum;

private:
    QList<Segment> createSegments(VectorXi const& edges, VectorXd const& xFreqs, VectorXd const& yFreqs, Response const& response) const;
    QList<Response> computeSpectrums(Response const& syncResponse, QList<Segment> const& segments) const;

private:
    QList<Response> const& mResponses;
};

}

#endif // HARMONICSOLVER_H
