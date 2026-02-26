#ifndef MATHUTILITY_H
#define MATHUTILITY_H

#include <Eigen/Core>
#include <QList>

using namespace Eigen;

namespace Backend::Core
{
class Response;
}

namespace Backend::Utility
{

// Statistics
double mean(VectorXd const& data);
double median(VectorXd const& data);
double standardDeviation(VectorXd const& data);
double meanAbsoluteDeviation(VectorXd const& data);
double medianAbsoluteDeviation(VectorXd const& data);

// Filter
VectorXi getIndices(std::vector<bool> const& data, bool value = true);
std::vector<bool> detectOutliers(VectorXd const& data, double threshold = 3);
VectorXd fillOutliers(VectorXd const& data);
std::pair<VectorXd, VectorXd> denoiseTotalVariance(VectorXd const& data, double lambda = 1, int numIter = 10);

// Roots
VectorXd findRoots(Backend::Core::Response const& response);
std::pair<VectorXd, VectorXd> evaluateFreqs(Backend::Core::Response const& response);

// Signal
VectorXd convolve(VectorXd const& a, VectorXd const& b);
VectorXcd hilbertTransform(VectorXd const& data);
std::vector<bool> unique(VectorXd const& data, double tolerance);
VectorXi sortIndices(VectorXd const& data);
}

#endif // MATHUTILITY_H
