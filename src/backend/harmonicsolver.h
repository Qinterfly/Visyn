#ifndef HARMONICSOLVER_H
#define HARMONICSOLVER_H

#include <QList>

#include "aliasdata.h"
#include "response.h"

namespace Backend::Core
{

struct Segment
{
    PairInt indices;
    PairDouble keys;
    double freq;
};

struct HarmonicOptions
{
    HarmonicOptions();
    ~HarmonicOptions() = default;

    int iSyncResponse;
    int iSyncSpectrum;
    double smoothFactor;
    int numIter;
    int numAverages;
    int numSkipPeriods;
    double maxFreq;
    double levelAmplitude;
    QList<PairDouble> intervals;
};

struct HarmonicSolution
{
    HarmonicSolution();
    ~HarmonicSolution() = default;

    bool isEmpty() const;
    void clear();

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
    QList<Response> refSpectrums;

private:
    bool checkResponses();
    QList<PairDouble> processIntervals(Backend::Core::Response const& response);
    QPair<VectorXd, VectorXd> evaluateFreqs(Backend::Core::Response const& response, QList<PairDouble> const& intervals);
    QList<Segment> createSegments(VectorXi const& edges, VectorXd const& xFreqs, VectorXd const& yFreqs, Response const& response) const;
    QList<Response> computeSpectrums(Response const& syncResponse, Response const& syncSpectrum, QList<Segment> const& segments) const;

private:
    QList<Response> const& mResponses;
};

}

#endif // HARMONICSOLVER_H
