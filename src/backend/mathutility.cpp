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
    result = std::sqrt(result / count);
    return result;
}

//! Compute median absolute deviation
double medianAbsoluteDeviation(VectorXd const& data)
{
    double medianValue = median(data);
    VectorXd absDeviation = (data.array() - medianValue).abs();
    return median(absDeviation);
}

//! Find outliers in data
std::vector<bool> detectOutliers(Eigen::VectorXd const& data, double threshold)
{
    // Constants
    double kFactor = 0.6745; // -1/(sqrt(2)*erfcinv(3/2))

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
        double score = kFactor * (data[i] - medianValue) / mad;
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
}
