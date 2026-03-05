#include <QDebug>
#include <QObject>

#include "edgedetector.h"
#include "harmonicsolver.h"
#include "mathutility.h"

using namespace Backend::Core;

static double const skEps = std::numeric_limits<double>::epsilon();

HarmonicOptions::HarmonicOptions()
    : iSyncResponse(-1)
    , iSyncSpectrum(-1)
    , smoothFactor(1e-1)
    , numIter(10)
    , numAverages(3)
    , numSkipPeriods(1)
    , maxFreq(512)
    , levelAmplitude(0.9)
{
    intervals.emplaceBack(PairDouble(0, -1));
}

HarmonicSolution::HarmonicSolution()
{
}

bool HarmonicSolution::isEmpty() const
{
    return segments.size() == 0;
}

void HarmonicSolution::clear()
{
    freqs = Response();
    filterFreqs = Response();
    segments.clear();
    spectrums.clear();
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
    int numResponses = mResponses.size();
    for (int i = 0; i != numResponses; ++i)
    {
        Response const& response = mResponses[i];
        if (response.isComplex())
        {
            qWarning() << QObject::tr("The time response %1 is complex-valued and cannot be procesed").arg(response.props.name);
            return solution;
        }
    }

    // Get the synchronization response in time domain
    int iSyncResponse = options.iSyncResponse;
    if (iSyncResponse < 0 || iSyncResponse >= numResponses)
        iSyncResponse = numResponses - 1;
    Response const& syncResponse = mResponses[iSyncResponse];

    // Get the synchronization response in frequency domain
    Response syncSpectrum;
    int numSpectrums = refSpectrums.size();
    int iSyncSpectrum = options.iSyncSpectrum;
    if (numSpectrums > 0)
    {
        if (iSyncSpectrum < 0 || iSyncSpectrum >= numSpectrums)
            iSyncSpectrum = numSpectrums - 1;
        syncSpectrum = refSpectrums[iSyncSpectrum];
    }
    if (!syncSpectrum.isEmpty() && !syncSpectrum.isComplex())
    {
        qWarning() << QObject::tr("The reference spectrum %1 is real-valued and cannot be procesed").arg(syncSpectrum.props.name);
        return solution;
    }

    // Evaluate frequencies
    auto [xFreqs, yFreqs] = evaluateFreqs(syncResponse);

    // Remove outliers from frequencies and fill them linearly
    yFreqs = Utility::fillOutliers(yFreqs);

    // Filter frequencies
    auto [yFilterFreqs, cost] = Utility::denoiseTotalVariance(yFreqs, options.smoothFactor, options.numIter);

    // Detect edges
    EdgeDetector detector(yFilterFreqs);
    VectorXi edges = detector.detect();

    // Create segments
    QList<Segment> segments = createSegments(edges, xFreqs, yFilterFreqs, syncResponse);

    // Compute spectrums
    QList<Response> spectrums = computeSpectrums(syncResponse, syncSpectrum, segments);

    // Set the solution
    solution.freqs = Response(xFreqs, yFreqs);
    solution.filterFreqs = Response(xFreqs, yFilterFreqs);
    solution.segments = segments;
    solution.spectrums = spectrums;

    return solution;
}

//! Estimate frequencies of harmonic process using its roots
QPair<VectorXd, VectorXd> HarmonicSolver::evaluateFreqs(Response const& response)
{
    // Find roots
    auto roots = Utility::findRoots(response);

    // Estimate the maximum frequency
    double maxFreq = options.maxFreq;
    if (maxFreq < skEps)
        maxFreq = response.props.sampleRate / 2.0;

    // Determine the maximum amplitude
    VectorXd const& yData = response.realValues();
    double maxAmplitude = yData.cwiseAbs().maxCoeff();
    double limitAmplitude = options.levelAmplitude * maxAmplitude;

    // Compute frequencies
    int numRoots = roots.size();
    VectorXd freqs(numRoots);
    QList<bool> mask(numRoots, false);
    int numFreqs = 0;
    for (int iRoot = 0; iRoot != numRoots - 1; ++iRoot)
    {
        auto currRoot = roots[iRoot];
        auto nextRoot = roots[iRoot + 1];
        double delta = nextRoot.x - currRoot.x;
        double amplitude = 0.0;
        for (int iData = currRoot.ind; iData != nextRoot.ind; ++iData)
            amplitude = std::max(amplitude, abs(yData[iData]));
        if (delta > skEps)
        {
            freqs[iRoot] = 0.5 / delta;
            mask[iRoot] = true;
        }
        if (freqs[iRoot] > maxFreq)
            mask[iRoot] = false;
        if (limitAmplitude > skEps && amplitude < limitAmplitude)
            mask[iRoot] = false;
        if (mask[iRoot])
            ++numFreqs;
    }

    // Copy valid frequencies
    VectorXd xResult(numFreqs);
    VectorXd yResult(numFreqs);
    numFreqs = 0;
    for (int i = 0; i != numRoots; ++i)
    {
        if (mask[i])
        {
            xResult[numFreqs] = roots[i].x;
            yResult[numFreqs] = freqs[i];
            ++numFreqs;
        }
    }
    return {xResult, yResult};
}

//! Build up the segments out of the edges
QList<Segment> HarmonicSolver::createSegments(VectorXi const& edges, VectorXd const& xFreqs, VectorXd const& yFreqs,
                                              Response const& response) const
{
    // Add both ends to indices
    int numIndices = edges.size() + 2;
    VectorXi indices(numIndices);
    indices << 0, edges, yFreqs.size() - 1;

    // Build up the segments
    QList<Segment> segments(numIndices - 1);
    int numSegments = 0;
    int iStart = indices[0];
    for (int i = 0; i != numIndices - 1; ++i)
    {
        // Get boundary values
        int iEnd = indices[i + 1];
        double xStart = xFreqs[iStart];
        double xEnd = xFreqs[iEnd];

        // Create the segment
        Segment segment;
        segment.indices = {response.index(xStart), response.index(xEnd)};
        segment.keys = {xStart, xEnd};
        segment.freq = Utility::median(yFreqs(seq(iStart, iEnd)));

        // Discard segments which do not contain one period
        if (segment.freq > skEps)
        {
            double period = 1.0 / segment.freq;
            double requestPeriod = period * (options.numAverages + options.numSkipPeriods);
            if (requestPeriod > xEnd - xStart)
                continue;
        }

        // Insert the new segment
        segments[numSegments] = segment;

        // Increase counters
        iStart = iEnd;
        ++numSegments;
    }
    segments.resize(numSegments);

    return segments;
}

//! Compute spectrums for time signals per each segment
QList<Response> HarmonicSolver::computeSpectrums(Response const& syncResponse, Response const& syncSpectrum, QList<Segment> const& segments) const
{
    // Constants
    double const kTolerance = 1e-6;

    // Get dimensions
    int numSegments = segments.size();
    int numResponses = mResponses.size();

    // Process all the segments
    VectorXd freqs(numSegments);
    MatrixXcd harmonicData(numSegments, numResponses);
    harmonicData.fill(0.0);
    for (int iSegment = 0; iSegment != numSegments; ++iSegment)
    {
        Segment const& segment = segments[iSegment];

        // Acquire segment data
        int iSegmentStart = segment.indices.first;
        int iSegmentEnd = segment.indices.second;
        double freq = segment.freq;
        freqs[iSegment] = freq;
        if (freq < skEps)
            continue;
        double period = 1.0 / freq;

        // Set the shift
        int iShift = ceil(period * syncResponse.props.sampleRate);
        int iCurrEnd = iSegmentEnd - options.numSkipPeriods * iShift;
        int iCurrStart = iCurrEnd - iShift;

        // Acquire the phase shift
        double shiftPhase = 0.0;
        if (!syncSpectrum.isEmpty())
        {
            int iFreq = syncSpectrum.index(freq);
            shiftPhase = syncSpectrum.phases()[iFreq];
        }

        // Process all the averages
        VectorXi countAverages(numResponses);
        countAverages.fill(0);
        for (int iPeriod = 0; iPeriod != options.numAverages; ++iPeriod)
        {
            if (iCurrStart < iSegmentStart || iCurrEnd < iSegmentStart || iCurrStart == iCurrEnd)
                break;

            // Slice sync data
            VectorXd ySyncData = syncResponse.realValues()(seq(iCurrStart, iCurrEnd));

            // Compute sin/cos for convolution
            VectorXd refSin = ySyncData.array() / ySyncData.maxCoeff();
            VectorXcd transform = Utility::hilbertTransform(refSin);
            VectorXd refCos = transform.imag();

            // Process all the responses
            for (int iResponse = 0; iResponse != numResponses; ++iResponse)
            {
                VectorXd yData = mResponses[iResponse].realValues()(seq(iCurrStart, iCurrEnd));
                int numData = yData.size();

                // Compute the instant phase
                double I = 0.0;
                double Q = 0.0;
                for (int iData = 0; iData != numData; ++iData)
                {
                    I += yData[iData] * refSin[iData];
                    Q += yData[iData] * refCos[iData];
                }
                I *= 2.0 / numData;
                Q *= 2.0 / numData;
                double instPhase = std::atan2(I, Q);

                // Refine the real and imaginary parts
                double A = std::sqrt(std::pow(I, 2.0) + std::pow(Q, 2.0));
                I = A * sin(instPhase + shiftPhase);
                Q = A * cos(instPhase + shiftPhase);

                // Save the data
                countAverages[iResponse] += 1;
                harmonicData(iSegment, iResponse) += std::complex<double>(I, -Q);
            }

            // Shift indices
            iCurrEnd = iCurrStart - 1;
            iCurrStart = iCurrEnd - iShift;
        }

        // Average the results
        for (int iResponse = 0; iResponse != numResponses; ++iResponse)
        {
            int numAvg = countAverages[iResponse];
            if (numAvg > 0)
                harmonicData(iSegment, iResponse) /= numAvg;
        }
    }

    // Slice by unique frequencies
    std::vector<bool> maskUnique = Utility::unique(freqs, kTolerance);
    VectorXi indices = Utility::getIndices(maskUnique);
    freqs = freqs(indices).eval();
    harmonicData = harmonicData(indices, indexing::all).eval();

    // Sort by frequencies
    indices = Utility::sortIndices(freqs);
    freqs = freqs(indices).eval();
    harmonicData = harmonicData(indices, indexing::all).eval();

    // Set the spectrums
    QList<Response> spectrums(numResponses);
    for (int iResponse = 0; iResponse != numResponses; ++iResponse)
    {
        Response& spectrum = spectrums[iResponse];
        spectrum.props = mResponses[iResponse].props;
        spectrum.props.domain = Domain::kFreq;
        spectrum.setKeys(freqs);
        spectrum.setValues((VectorXcd) harmonicData(indexing::all, iResponse));
    }

    return spectrums;
}
