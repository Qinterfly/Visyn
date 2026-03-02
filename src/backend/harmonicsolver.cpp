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
    , smoothFactor(1e-3)
    , numIter(10)
    , numAverages(3)
    , numSkipPeriods(1)
{
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

    // Fix up synchronization channel index in time domain
    int numResponses = mResponses.size();
    if (options.iSyncResponse < 0 || options.iSyncResponse >= numResponses)
        options.iSyncResponse = numResponses - 1;
    Response const& syncResponse = mResponses[options.iSyncResponse];

    // Fix up synchronization channel index in frequency domain
    Response syncSpectrum;
    int numSpectrums = refSpectrums.size();
    if (numSpectrums > 0)
    {
        if (options.iSyncSpectrum < 0 || options.iSyncSpectrum >= numSpectrums)
            options.iSyncSpectrum = numSpectrums - 1;
        syncSpectrum = refSpectrums[options.iSyncSpectrum];
    }

    // Evaluate frequencies
    auto [xFreqs, yFreqs] = Utility::evaluateFreqs(syncResponse);

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

//! Build up the segments out of the edges
QList<Segment> HarmonicSolver::createSegments(VectorXi const& edges, VectorXd const& xFreqs, VectorXd const& yFreqs,
                                              Response const& response) const
{
    // Add both ends to indices
    int numIndices = edges.size() + 2;
    VectorXi indices(numIndices);
    indices << 0, edges, yFreqs.size() - 1;

    // Build up the segments
    int numSegments = numIndices - 1;
    QList<Segment> segments(numSegments);
    for (int i = 0; i != numSegments; ++i)
    {
        Segment segment;
        int iStart = indices[i];
        int iEnd = indices[i + 1];
        double xStart = xFreqs[iStart];
        double xEnd = xFreqs[iEnd];
        segment.indices = {response.index(xStart), response.index(xEnd)};
        segment.keys = {xStart, xEnd};
        segment.freq = Utility::median(yFreqs(seq(iStart, iEnd)));
        segments[i] = segment;
    }
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
            if (iCurrStart < iSegmentStart || iCurrEnd < iSegmentStart)
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
