#include <QEvent>

#include "customdoublespinbox.h"

CustomDoubleSpinBox::CustomDoubleSpinBox(QWidget* pParent)
    : QDoubleSpinBox(pParent)
    , mDecimalPoint(".")
    , mIsWheelEnabled(true)
{
    installEventFilter(this);
}

CustomDoubleSpinBox::~CustomDoubleSpinBox()
{

}

QValidator::State CustomDoubleSpinBox::validate(QString& text, int& pos) const
{
    QList<QString> const replacement = {".", ",", "б", "ю"};
    for (auto const& item : replacement)
        if (item.compare(mDecimalPoint, Qt::CaseInsensitive) != 0)
            text = text.replace(item, mDecimalPoint, Qt::CaseInsensitive);
    return QDoubleSpinBox::validate(text, pos);
}

bool CustomDoubleSpinBox::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Wheel && !mIsWheelEnabled)
        return true;
    return false;
}

void CustomDoubleSpinBox::setWheelEnabled(bool flag)
{
    mIsWheelEnabled = flag;
}
