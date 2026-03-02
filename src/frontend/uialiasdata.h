#ifndef UIALIASDATA_H
#define UIALIASDATA_H

#include <Eigen/Geometry>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QLineEdit)

namespace Frontend
{

class IntLineEdit;
class DoubleLineEdit;
using Edit1s = QLineEdit;
using Edit1i = IntLineEdit;
using Edit1d = DoubleLineEdit;
}

#endif // UIALIASDATA_H
