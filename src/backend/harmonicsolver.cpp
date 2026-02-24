#include <QDebug>
#include <QObject>

#include "harmonicsolver.h"
#include "mathutility.h"
#include "response.h"

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

    // Fix up the synchronization channel index
    int numResponses = mResponses.size();
    if (options.iSync < 0 || options.iSync >= numResponses)
        options.iSync = numResponses - 1;
    Response const& syncResponse = mResponses[options.iSync];

    // Evaluate the frequencies
    evaluateFreqs(syncResponse);

    // Filter the frequencies
    Utility::fillOutliers(mYFreqs);

    return solution;
}

//! Use the reference response to evaluate process frequencies
void HarmonicSolver::evaluateFreqs(Response const& response)
{
    // Find all the roots
    VectorXd xFreqs = Utility::findRoots(response);

    // Estimate the Nyquist frequenciy
    double nyquistFreq = response.props.sampleRate / 2.0;

    // Compute the frequencies
    int numRoots = xFreqs.size();
    VectorXd yFreqs(numRoots);
    std::vector<bool> mask(numRoots, false);
    int numFreqs = 0;
    for (int i = 0; i != numRoots - 1; ++i)
    {
        double delta = xFreqs[i + 1] - xFreqs[i];
        if (delta > skEps)
        {
            yFreqs[i] = 0.5 / delta;
            mask[i] = true;
        }
        if (yFreqs[i] > nyquistFreq)
            mask[i] = false;
        ++numFreqs;
    }

    // Copy the valid frequencies
    mXFreqs.resize(numFreqs);
    mYFreqs.resize(numFreqs);
    numFreqs = 0;
    for (int i = 0; i != numRoots; ++i)
    {
        if (mask[i])
        {
            mXFreqs[numFreqs] = xFreqs[i];
            mYFreqs[numFreqs] = yFreqs[i];
            ++numFreqs;
        }
    }
}
