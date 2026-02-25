#include "mathutility.h"
#include "response.h"

using namespace Backend::Core;

namespace Backend::Utility
{

//! Compute total mean value
double mean(Eigen::VectorXd const& data)
{
    int count = data.size();
    if (count == 0)
        return 0.0;
    return std::accumulate(data.begin(), data.end(), 0.0) / count;
}

//! Compute a median value
double median(Eigen::VectorXd const& data)
{
    Eigen::VectorXd series = data;
    std::sort(series.data(), series.data() + series.size());
    int count = series.size();
    int iMiddle = count / 2;
    if (count % 2 == 0)
        return 0.5 * (series(iMiddle - 1) + series(iMiddle));
    else
        return series(iMiddle);
}

//! Compute standard deviation value
double standardDeviation(VectorXd const& data)
{
    double result = 0.0;
    int count = data.size();
    if (count <= 1)
        return result;
    double meanValue = mean(data);
    for (int i = 0; i != count; ++i)
        result += std::pow((data[i] - meanValue), 2.0);
    result = std::sqrt(result / (count - 1));
    return result;
}

//! Compute mean absolute deviation
double meanAbsoluteDeviation(VectorXd const& data)
{
    double meanValue = mean(data);
    VectorXd absDev = (data.array() - meanValue).abs();
    return mean(absDev);
}

//! Compute median absolute deviation
double medianAbsoluteDeviation(VectorXd const& data)
{
    double medianValue = median(data);
    VectorXd absDev = (data.array() - medianValue).abs();
    return median(absDev);
}

//! Get indices of elements which equal flag
VectorXi getIndices(std::vector<bool> const& data, bool value)
{
    int numData = data.size();
    int numIndices = 0;
    for (int i = 0; i != numData; ++i)
    {
        if (data[i] == value)
            ++numIndices;
    }
    VectorXi indices(numIndices);
    numIndices = 0;
    for (int i = 0; i != numData; ++i)
    {
        if (data[i] == value)
        {
            indices[numIndices] = i;
            ++numIndices;
        }
    }
    return indices;
}

//! Find outliers in data
std::vector<bool> detectOutliers(VectorXd const& data, double threshold)
{
    // Constants
    double kSigma = 0.6745; // -1/(sqrt(2)*erfcinv(3/2))

    // Allocate the resulting mask
    int count = data.size();
    std::vector<bool> result(count, false);

    // Estimate chararteristics
    double medianValue = median(data);
    double mad = medianAbsoluteDeviation(data);
    if (mad < std::numeric_limits<double>::epsilon())
        return result;

    // Mark the outliers
    for (int i = 0; i != count; ++i)
    {
        double score = kSigma * (data[i] - medianValue) / mad;
        if (std::abs(score) > threshold)
            result[i] = true;
    }

    return result;
}

//! Fill outliers in data
VectorXd fillOutliers(VectorXd const& data)
{
    // Find the outliers
    std::vector<bool> outliers = detectOutliers(data);

    // Fill the outliers
    int count = data.size();
    VectorXd result(count);
    for (int iCurr = 0; iCurr != count; ++iCurr)
    {
        double value = data[iCurr];
        if (outliers[iCurr])
        {
            // Find the previous nonoutlier
            int iPrev = iCurr - 1;
            while (iPrev >= 0 && outliers[iPrev])
                --iPrev;

            // Find the next nonoutlier
            int iNext = iCurr + 1;
            while (iNext < count && outliers[iPrev])
                ++iNext;

            // Interpolate
            bool isPrev = iPrev >= 0;
            bool isNext = iNext < count;
            if (isPrev && isNext)
            {
                double t = double(iCurr - iPrev) / (iNext - iPrev);
                value = data[iPrev] * (1.0 - t) + data[iNext] * t;
            }
            // Or take the previous value
            else if (isPrev)
            {
                value = data[iPrev];
            }
            // Or take the next value
            else if (isNext)
            {
                value = data[iNext];
            }
        }
        result[iCurr] = value;
    }

    return result;
}

//! Total variation denoising using majorization-minimization and banded linear systems
std::pair<VectorXd, VectorXd> denoiseTotalVariance(VectorXd const& data, double lambda, int numIter)
{
    VectorXd const& y = data;

    // Get dimensions
    int N = y.size();
    int M = N - 1;

    // Precompute first differences
    VectorXd Dy(M);
    for (int i = 0; i < M; ++i)
        Dy(i) = y(i + 1) - y(i);

    // Allocate working arrays
    VectorXd d(M);
    VectorXd b(M);
    VectorXd rhs(M);
    VectorXd u(M);
    VectorXd x = y;
    VectorXd Dx = Dy;

    // Allocate cost function
    VectorXd cost(numIter);

    // Iterate
    for (int iIter = 0; iIter != numIter; ++iIter)
    {
        // Update diagonal increment
        d = Dx.array().abs() / lambda;

        // Setup tridiagonal system: F = diag(2 + d) + tridiag(-1, -1)
        b = 2.0 + d.array();

        // Solve F * u = Dy using Thomas algorithm
        rhs = Dy;

        // Forward elimination
        for (int i = 1; i < M; ++i)
        {
            double w = -1.0 / b(i - 1);
            b(i) = b(i) + w;
            rhs(i) = rhs(i) - w * rhs(i - 1);
        }

        // Back substitution
        u(M - 1) = rhs(M - 1) / b(M - 1);
        for (int i = M - 2; i >= 0; --i)
            u(i) = (rhs(i) + u(i + 1)) / b(i);

        // Update x = y - D' * u
        x(0) = y(0) + u(0);
        for (int i = 1; i < M; ++i)
            x(i) = y(i) - (u(i - 1) - u(i));
        x(N - 1) = y(N - 1) - u(M - 1);

        // Update Dx = D * x
        for (int i = 0; i < M; ++i)
            Dx(i) = x(i + 1) - x(i);

        // Compute cost: 0.5 * ||x - y|| ^ 2 + lambda * sum(|Dx|)
        double term1 = 0.5 * (x - y).squaredNorm();
        double term2 = lambda * Dx.array().abs().sum();
        cost[iIter] = term1 + term2;
    }
    return {x, cost};
}

//! Find all the response roots
VectorXd findRoots(Response const& response)
{
    auto const& values = response.realValues();
    int numValues = response.numValues();
    VectorXd roots;
    roots.resize(numValues);
    int numRoots = 0;
    for (int i = 0; i != numValues - 1; ++i)
    {
        double y1 = values[i];
        double y2 = values[i + 1];
        if (y1 * y2 < 0.0)
        {
            double x1 = response.key(i);
            double x2 = response.key(i + 1);
            double root = x1 - y1 * (x2 - x1) / (y2 - y1);
            roots[numRoots] = root;
            ++numRoots;
        }
    }
    roots.conservativeResize(numRoots);
    return roots;
}

//! Estimate frequencies of harmonic process using its roots
std::pair<VectorXd, VectorXd> evaluateFreqs(Response const& response)
{
    // Find roots
    VectorXd roots = findRoots(response);

    // Estimate the Nyquist frequenciy
    double nyquistFreq = response.props.sampleRate / 2.0;

    // Compute frequencies
    int numRoots = roots.size();
    VectorXd freqs(numRoots);
    std::vector<bool> mask(numRoots, false);
    int numFreqs = 0;
    for (int i = 0; i != numRoots - 1; ++i)
    {
        double delta = roots[i + 1] - roots[i];
        if (delta > std::numeric_limits<double>::epsilon())
        {
            freqs[i] = 0.5 / delta;
            mask[i] = true;
        }
        if (freqs[i] > nyquistFreq)
            mask[i] = false;
        if (mask[i])
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
            xResult[numFreqs] = roots[i];
            yResult[numFreqs] = freqs[i];
            ++numFreqs;
        }
    }
    return {xResult, yResult};
}

//! Perform full convolution
VectorXd convolve(VectorXd const& a, VectorXd const& b)
{
    // Get dimensions
    int Na = a.size();
    int Nb = b.size();
    int N = Na + Nb - 1;

    // Allocate the result
    VectorXd result(N);
    result.setZero();

    // Convolve
    for (int i = 0; i != Na; ++i)
        for (int j = 0; j != Nb; ++j)
            result(i + j) += a(i) * b(j);
    return result;
}
}
