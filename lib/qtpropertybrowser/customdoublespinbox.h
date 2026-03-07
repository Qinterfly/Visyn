#ifndef CUSTOMDOUBLESPINBOX_H
#define CUSTOMDOUBLESPINBOX_H

#include <QDoubleSpinBox>

class CustomDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    CustomDoubleSpinBox(QWidget* pParent = nullptr);
    virtual ~CustomDoubleSpinBox();

    QValidator::State validate(QString& text, int& pos) const override;
    bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    void setWheelEnabled(bool flag);

private:
    QString mDecimalPoint;
    bool mIsWheelEnabled;
};

#endif // CUSTOMDOUBLESPINBOX_H
