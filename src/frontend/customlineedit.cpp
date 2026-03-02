#include <QDoubleValidator>
#include <QKeyEvent>

#include "customlineedit.h"

using namespace Frontend;

IntLineEdit::IntLineEdit(QWidget* pParent)
    : QLineEdit(pParent)
{
    mpValidator = new QIntValidator(this);
    setValidator(mpValidator);
    connect(this, &IntLineEdit::editingFinished, this, &IntLineEdit::processEditingFinished);
}

IntLineEdit::IntLineEdit(int minimum, int maximum, QWidget* pParent)
    : IntLineEdit(pParent)
{
    setRange(minimum, maximum);
}

//! Get the current value
int IntLineEdit::value() const
{
    bool isOk = false;
    double result = text().toInt(&isOk);
    return isOk ? result : (mpValidator->bottom() + mpValidator->top()) / 2.0;
}

//! Get the lower value boundary
int IntLineEdit::minimum() const
{
    return mpValidator->bottom();
}

//! Get the upper value boundary
int IntLineEdit::maximum() const
{
    return mpValidator->top();
}

//! Set the current value
void IntLineEdit::setValue(int value)
{
    QString newText = QString::number(value);
    mpValidator->fixup(newText);
    if (newText != mPreviousText)
        setText(newText);
    mPreviousText = newText;
}

//! Set the lower value boundary
void IntLineEdit::setMinimum(int value)
{
    mpValidator->setBottom(value);
}

//! Set the upper value boundary
void IntLineEdit::setMaximum(int value)
{
    mpValidator->setTop(value);
}

//! Set the lower and upper value boundaries
void IntLineEdit::setRange(int minimum, int maximum)
{
    mpValidator->setRange(minimum, maximum);
}

//! Hide borders using stylesheet
void IntLineEdit::hideBorders()
{
    setStyleSheet(styleSheet().append("border: none;"));
}

//! Fixup the input if it has been rejected after return pressed
void IntLineEdit::keyPressEvent(QKeyEvent* pEvent)
{
    QLineEdit::keyPressEvent(pEvent);
    if (pEvent->key() == Qt::Key_Return)
    {
        int position = 0;
        QString newText = text();
        auto state = mpValidator->validate(newText, position);
        if (state == QValidator::Acceptable)
            return;
        QSignalBlocker blocker(this);
        setText(mPreviousText);
    }
}

//! Slot function to handle termination of editing
void IntLineEdit::processEditingFinished()
{
    QString newText = text();
    if (mPreviousText != newText)
        emit valueChanged();
    mPreviousText = newText;
}

DoubleLineEdit::DoubleLineEdit(QWidget* pParent)
    : QLineEdit(pParent)
{
    int const kNumDecimals = 4;
    double const kRangeValue = 1e9;

    mpValidator = new QDoubleValidator(this);
    mpValidator->setNotation(QDoubleValidator::ScientificNotation);
    mpValidator->setRange(-kRangeValue, kRangeValue, kNumDecimals);
    mpValidator->setLocale(QLocale::C);
    setValidator(mpValidator);
    connect(this, &DoubleLineEdit::textEdited, this, &DoubleLineEdit::processTextEdited);
    connect(this, &DoubleLineEdit::editingFinished, this, &DoubleLineEdit::processEditingFinished);
}

DoubleLineEdit::DoubleLineEdit(double minimum, double maximum, int decimals, QWidget* pParent)
    : DoubleLineEdit(pParent)
{
    setRange(minimum, maximum);
    setDecimals(decimals);
}

//! Get the current value
double DoubleLineEdit::value() const
{
    bool isOk = false;
    double result = text().toDouble(&isOk);
    return isOk ? result : (mpValidator->bottom() + mpValidator->top()) / 2.0;
}

//! Retrieve the minimum accepted value
double DoubleLineEdit::minimum() const
{
    return mpValidator->bottom();
}

//! Retrieve the maximum accepted value
double DoubleLineEdit::maximum() const
{
    return mpValidator->top();
}

//! Retrieve the number of decimals
int DoubleLineEdit::decimals() const
{
    return mpValidator->decimals();
}

//! Set the current value
void DoubleLineEdit::setValue(double value)
{
    if (isReadOnly())
        return;
    QString newText = QString::number(value, 'g', 6);
    if (mPreviousText != newText)
        setText(newText);
    mPreviousText = newText;
}

//! Set the lower value boundary
void DoubleLineEdit::setMinimum(double value)
{
    mpValidator->setBottom(value);
}

//! Set the upper value boundary
void DoubleLineEdit::setMaximum(double value)
{
    mpValidator->setTop(value);
}

//! Set value boundaries
void DoubleLineEdit::setRange(double minimum, double maximum)
{
    mpValidator->setRange(minimum, maximum);
}

//! Set number of decimals of text representation
void DoubleLineEdit::setDecimals(int number)
{
    mpValidator->setDecimals(number);
}

//! Hide borders using stylesheet
void DoubleLineEdit::hideBorders()
{
    setStyleSheet(styleSheet().append("border: none;"));
}

//! Fixup the input if it has been rejected after return pressed
void DoubleLineEdit::keyPressEvent(QKeyEvent* pEvent)
{
    QLineEdit::keyPressEvent(pEvent);
    if (pEvent->key() == Qt::Key_Return)
    {
        int position = 0;
        QString newText = text();
        auto state = mpValidator->validate(newText, position);
        if (state == QValidator::Acceptable)
            return;
        QSignalBlocker blocker(this);
        setText(mPreviousText);
    }
}

//! Slot function to handle text changes
void DoubleLineEdit::processTextEdited()
{
    int position = 0;
    QString newText = text();
    if (newText.contains(","))
    {
        QSignalBlocker blocker(this);
        newText.replace(",", ".");
        setText(newText);
        mPreviousText = newText;
    }
}

//! Slot function to handle termination of editing
void DoubleLineEdit::processEditingFinished()
{
    QString newText = text();
    if (mPreviousText != newText)
        emit valueChanged();
    mPreviousText = newText;
}
