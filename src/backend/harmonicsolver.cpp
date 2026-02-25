#include <QDebug>
#include <QObject>

#include "edgedetector.h"
#include "harmonicsolver.h"
#include "mathutility.h"

using namespace Backend::Core;

static double const skEps = std::numeric_limits<double>::epsilon();

HarmonicOptions::HarmonicOptions()
    : iSync(-1)
    , numIter(10)
    , smoothFactor(1e-3)
{
}

HarmonicSolver::HarmonicSolver(QList<Response> const& responses)
    : mResponses(responses)
{
}

HarmonicSolution HarmonicSolver::solve()
{
    HarmonicSolution solution;

    // Sanity check
    if (mResponses.empty())
    {
        qWarning() << QObject::tr("There are no responses to obtain harmonic solution");
        return solution;
    }

    // Fix up synchronization channel index
    int numResponses = mResponses.size();
    if (options.iSync < 0 || options.iSync >= numResponses)
        options.iSync = numResponses - 1;
    Response const& syncResponse = mResponses[options.iSync];

    // Evaluate frequencies
    auto [xFreqs, yFreqs] = Utility::evaluateFreqs(syncResponse);

    // Remove outliers from frequencies and fill them linearly
    yFreqs = Utility::fillOutliers(yFreqs);
    solution.freqs = Response(xFreqs, yFreqs);

    // Filter frequencies
    auto [yFilterFreqs, cost] = Utility::denoiseTotalVariance(yFreqs, options.smoothFactor, options.numIter);
    solution.filterFreqs = Response(xFreqs, yFilterFreqs);

    // Detect edges
    EdgeDetector detector(solution.filterFreqs);
    VectorXi edges = detector.detect();

    return solution;
}
