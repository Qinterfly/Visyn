#define _USE_MATH_DEFINES
#include <cmath>

#include "edgedetector.h"
#include "mathutility.h"

using namespace Backend::Core;

static double const skEps = std::numeric_limits<double>::epsilon();

EdgeDetector::EdgeDetector(Response const& response)
    : order(1)
    , width(3)
    , thresholds(4)
    , scales(4)
    , mResponse(response)
{
    thresholds << 0.1, 0.2, 0.3, 0.4;
    scales << 1, 2, 4, 8;
}

//! Find all the edges of the response
VectorXi EdgeDetector::detect()
{
    // Create the derivative scale space
    // minima and maxima of the derivative correspond to transitions
    std::vector<VectorXd> dData = createGaussScaleSpace();

    // Find the position of local minima and maxima of the most coarse scale
    std::vector<bool> minmax = findLocalExtrema(dData.back(), thresholds(indexing::last), scales(indexing::last));
    VectorXi minmaxIndices = Utility::getIndices(minmax);

    // Refine min/max positions through scale space
    int numScales = scales.size();
    for (int i = numScales - 2; i >= 0; --i)
    {
        minmax = findLocalExtrema(dData[i], thresholds[i], scales[i], minmaxIndices);
        minmaxIndices = Utility::getIndices(minmax);
    }
    return minmaxIndices;
}

//! Computes the Gaussian scale space
std::vector<VectorXd> EdgeDetector::createGaussScaleSpace() const
{
    VectorXd const& data = mResponse.realValues();
    int numData = data.size();
    int numScales = scales.size();

    // Loop through all the scales
    std::vector<VectorXd> space(numScales);
    for (int iScale = 0; iScale != numScales; ++iScale)
    {
        // Find the gaussian kernel
        VectorXd kernel = createGaussKernel(scales[iScale]);

        // We have to pad the data to avoid the derivative blowing up at the boundaries
        int numKernel = kernel.size();
        VectorXd padData(numData + 2 * numKernel);
        for (int k = 0; k != numKernel; ++k)
        {
            padData[k] = data[0];
            padData[numKernel + numData + k] = data[numData - 1];
        }
        for (int k = 0; k != numData; ++k)
            padData[numKernel + k] = data[k];

        // Convolve
        VectorXd fData = Utility::convolve(padData, kernel);

        // Get filtered data
        int offset = ceil((fData.size() - data.size()) / 2.0) - 1;
        space[iScale] = fData(seqN(offset, numData));
    }
    return space;
}

//! Create the gaussian kernel
VectorXd EdgeDetector::createGaussKernel(int scale) const
{
    // Initialize distribution range
    double sigma = scale;
    int numRange = 2 * width * (ceil(sigma)) + 1;
    VectorXd range = VectorXd::LinSpaced(numRange, 1, numRange);
    double center = range[ceil(numRange / 2.0) - 1];

    // Estimate derivatives
    VectorXd derivs(numRange);
    VectorXd diffs = range.array() - center;
    double sigma2 = std::pow(sigma, 2.0);
    double sigma4 = std::pow(sigma, 4.0);
    switch (order)
    {
    case 0:
        derivs.fill(1.0);
        break;
    case 1:
        derivs = -diffs / sigma2;
        break;
    case 2:
        derivs = (diffs.cwisePow(2.0).array() - sigma2) / sigma4;
        break;
    default:
        derivs.fill(0.0);
        break;
    }

    // Compute kernel
    VectorXd kernel(numRange);
    double c = 1.0 / (sigma * sqrt(2.0 * M_PI));
    for (int i = 0; i != numRange; ++i)
    {
        double value = c * exp(-std::pow(diffs[i], 2.0) / (2.0 * sigma2));
        kernel[i] = value * derivs[i];
    }

    return kernel;
}

//! Find distinct transition regions and transition ratios
std::vector<bool> EdgeDetector::findLocalExtrema(VectorXd const& data, double threshold, int scale, VectorXi const& regions) const
{
    int numData = data.size();

    // Rescale data
    double maxVal = data.maxCoeff();
    double minVal = data.minCoeff();
    VectorXd rdataMax = std::abs(maxVal) > skEps ? (VectorXd) (data / maxVal) : VectorXd::Zero(numData);
    VectorXd rdataMin = std::abs(minVal) > skEps ? (VectorXd) (data / minVal) : VectorXd::Zero(numData);

    // Sliding window maxima of the normalized data
    VectorXd winmax = VectorXd::Zero(numData);
    VectorXd winmin = VectorXd::Zero(numData);

    // Compute moving max and min values
    int iStart = scale;
    int iEnd = numData - 1 - scale;
    if (iStart <= iEnd)
    {
        for (int i = iStart; i <= iEnd; ++i)
        {
            winmax[i] = rdataMax.segment(i - scale, 2 * scale + 1).maxCoeff();
            winmin[i] = rdataMin.segment(i - scale, 2 * scale + 1).minCoeff();
        }
    }

    // Find the local minima and maxima
    std::vector<bool> maxima(numData, false);
    std::vector<bool> minima(numData, false);
    int numRegions = regions.size();
    if (numRegions == 0)
    {
        // Search everywhere
        for (int i = iStart; i <= iEnd; ++i)
        {
            if (rdataMax[i] >= threshold && rdataMax[i] >= winmax[i])
                maxima[i] = true;
            if (rdataMin[i] >= threshold && rdataMin[i] >= winmin[i])
                minima[i] = true;
        }
    }
    else
    {
        // Search only near given region centers
        for (int r = 0; r != numRegions; ++r)
        {
            int center = regions[r];
            int low = std::max(iStart, center - scale);
            int high = std::min(iEnd, center + scale);
            for (int i = low; i <= high; ++i)
            {
                if (rdataMax[i] >= threshold && rdataMax[i] >= winmax[i])
                    maxima[i] = true;
                if (rdataMin[i] >= threshold && rdataMin[i] >= winmin[i])
                    minima[i] = true;
            }
        }
    }

    // Set the result
    std::vector<bool> extrema(numData);
    for (int i = 0; i != numData; ++i)
        extrema[i] = maxima[i] || minima[i];

    return extrema;
}
