#ifndef EDGEDETECTOR_H
#define EDGEDETECTOR_H

#include "response.h"

namespace Backend::Core
{

//! Class which uses Canny edge detection algorithm to find abrupt changes in signal
class EdgeDetector
{
public:
    EdgeDetector(Response const& response);
    ~EdgeDetector() = default;

    VectorXi detect();

public:
    int order;
    int width;
    VectorXd thresholds;
    VectorXi scales;

private:
    std::vector<VectorXd> createGaussScaleSpace() const;
    VectorXd createGaussKernel(int scale) const;
    std::vector<bool> findLocalExtrema(VectorXd const& data, double threshold, int scale, VectorXi const& regions = {}) const;

private:
    Response const& mResponse;
};

}

#endif // EDGEDETECTOR_H
