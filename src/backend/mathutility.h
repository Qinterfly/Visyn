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

double mean(VectorXd const& data);
double median(VectorXd const& data);
double standardDeviation(VectorXd const& data);
double medianAbsoluteDeviation(VectorXd const& data);
std::vector<bool> detectOutliers(VectorXd const& data, double threshold = 3);
VectorXd fillOutliers(VectorXd const& data);
VectorXd findRoots(Backend::Core::Response const& response);
}

#endif // MATHUTILITY_H
