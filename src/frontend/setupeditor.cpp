#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "customtable.h"
#include "fileutility.h"
#include "response.h"
#include "setupeditor.h"
#include "uiutility.h"

using namespace Backend;
using namespace Backend::Core;
using namespace Frontend;

ResponseProperties buildProperties(QStringList const& fields, QStringList const& values);
QStringList rowValues(CustomTable* pTable, int iRow);
void writeLine(QTextStream& stream, QChar separator, QStringList const& values);

SetupEditor::SetupEditor(QSettings& settings, QWidget* pParent)
    : QWidget(pParent)
    , mSettings(settings)
{
    setWindowTitle(tr("Setup"));
    createContent();
    refresh();
}

SetupEditor::~SetupEditor()
{
}

QSize SetupEditor::sizeHint() const
{
    return QSize(800, 600);
}

QString SetupEditor::fileSuffix() const
{
    return "csv";
}

QList<ResponseProperties> const& SetupEditor::setup() const
{
    return mSetup;
}

//! Replace properties of the responses using the current setup
void SetupEditor::replaceProperties(QList<Backend::Core::Response>& responses) const
{
    // Map the properties
    QMap<int, ResponseProperties> mapSetup;
    for (ResponseProperties const& props : mSetup)
        mapSetup[props.id] = props;

    // Set the properties
    for (Response& response : responses)
    {
        ResponseProperties& props = response.props;
        if (mapSetup.contains(props.id))
            props = mapSetup[props.id];

        // Define the name by default
        if (props.name.isEmpty())
        {
            QString sign = props.sign > 0 ? "+" : "-";
            QString prefix = response.isComplex() ? "Visyn Spectrum" : "Visyn Time";
            props.name = QString("%1 %2:%3:%4%5").arg(prefix, props.component, props.node, sign, getLabel(props.direction));
        }
    }
}

//! Write response setup to a file
bool SetupEditor::write(QString const& pathFile)
{
    // Open the file for writing
    auto pFile = Backend::Utility::openFile(pathFile, fileSuffix(), QIODevice::WriteOnly);
    if (!pFile)
        return false;
    QTextStream stream(pFile.data());

    // Set the separator
    QLocale locale;
    QChar separator;
    if (locale.decimalPoint() == '.')
        separator = ',';
    else
        separator = ';';

    // Write the header
    QStringList fields = mpDataTable->horizontalHeaderLabels();
    writeLine(stream, separator, fields);

    // Write the content
    int numData = mpDataTable->rowCount();
    for (int i = 0; i != numData; ++i)
    {
        QStringList values = rowValues(mpDataTable, i);
        writeLine(stream, separator, values);
    }

    return true;
}

//! Read response setup from a file
bool SetupEditor::read(QString const& pathFile)
{
    // Clear the previous properties
    mSetup.clear();
    refresh();

    // Open the file for reading
    auto pFile = Backend::Utility::openFile(pathFile, fileSuffix(), QIODevice::ReadOnly);
    if (!pFile)
        return false;

    // Read the fields
    QChar separator = ';';
    QTextStream stream(pFile.data());
    QString line = stream.readLine();
    if (!line.contains(separator))
        separator = ',';
    QStringList fields = line.split(separator, Qt::KeepEmptyParts);
    if (fields.isEmpty())
        return false;

    // Read the properties
    while (!stream.atEnd())
    {
        line = stream.readLine();

        // Parse the values
        QStringList values = line.split(separator, Qt::KeepEmptyParts);
        if (values.size() != fields.size())
            continue;

        // Set the properties
        ResponseProperties props = buildProperties(fields, values);
        mSetup.push_back(props);
    }

    // Update the widgets
    refresh();

    return true;
}

//! Update the widgets content
void SetupEditor::refresh()
{
    // Constants
    QStringList const kFields = {"Id", "Node", "Component", "Direction", "Sign", "Transducer", "Comment"};

    // Set the number of data
    QSignalBlocker blockerNumData(mpNumDataEdit);
    int numData = mSetup.size();
    int numFields = kFields.size();
    mpNumDataEdit->setValue(numData);

    // Set the table dimensions
    QSignalBlocker blockerDataTable(mpDataTable);
    mpDataTable->clearContents();
    mpDataTable->setHorizontalHeaderLabels(kFields);
    mpDataTable->setRowCount(numData);
    mpDataTable->setColumnCount(numFields);

    // Set the table data
    QMetaObject const& metaObject = ResponseProperties::staticMetaObject;
    for (int iData = 0; iData != numData; ++iData)
    {
        ResponseProperties const& props = mSetup[iData];
        for (int iField = 0; iField != numFields; ++iField)
        {
            // Get the property
            QString field = kFields[iField].toLower();
            int iProperty = metaObject.indexOfProperty(field.toStdString().data());
            if (iProperty == -1)
                continue;
            QMetaProperty property = metaObject.property(iProperty);
            QVariant value = property.readOnGadget(&props);
            if (field == "direction")
                value = getLabel((Direction) value.toInt());
            else if (field == "sign")
                value = value.toInt() > 0 ? "+" : "-";
            mpDataTable->setItem(iData, iField, Utility::createTableItem(value.toString()));
        }
    }
}

//! Create all the widgets and corresponding content
void SetupEditor::createContent()
{
    // Create all the widgets
    mpDataTable = new CustomTable;
    mpNumDataEdit = new Edit1i;
    QPushButton* pReadButton = new QPushButton(QIcon(":/icons/document-open.svg"), tr("&Import"));
    QPushButton* pWriteButton = new QPushButton(QIcon(":/icons/document-save-as.svg"), tr("&Export"));

    // Initialize the widgets
    mpNumDataEdit->setMinimum(0);
    mpDataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Set the connections
    connect(mpNumDataEdit, &Edit1i::valueChanged, this, &SetupEditor::setNumData);
    connect(mpDataTable, &CustomTable::itemChanged, this, &SetupEditor::setData);
    connect(pReadButton, &QPushButton::clicked, this, &SetupEditor::readDialog);
    connect(pWriteButton, &QPushButton::clicked, this, &SetupEditor::writeDialog);

    // Create the edit layout
    QHBoxLayout* pEditLayout = new QHBoxLayout;
    pEditLayout->addWidget(new QLabel(tr("Number of rows: ")));
    pEditLayout->addWidget(mpNumDataEdit);
    pEditLayout->addStretch(1);

    // Create the control layout
    QHBoxLayout* pControlLayout = new QHBoxLayout;
    pControlLayout->addStretch();
    pControlLayout->addWidget(pReadButton);
    pControlLayout->addWidget(pWriteButton);
    pControlLayout->addStretch();

    // Create the main layout
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addLayout(pEditLayout);
    pMainLayout->addWidget(mpDataTable);
    pMainLayout->addLayout(pControlLayout);
    setLayout(pMainLayout);
}

//! Resize the data
void SetupEditor::setNumData()
{
    int numData = mpNumDataEdit->value();
    mSetup.resize(numData);
    for (int i = 0; i != numData; ++i)
    {
        if (mSetup[i].id < 1)
            mSetup[i].id = 1 + i;
    }
    refresh();
}

//! Apply the changes
void SetupEditor::setData()
{
    QStringList fields = mpDataTable->horizontalHeaderLabels();
    int numData = mpDataTable->rowCount();

    // Slice the values from the table
    for (int i = 0; i != numData; ++i)
    {
        QStringList values = rowValues(mpDataTable, i);
        mSetup[i] = buildProperties(fields, values);
    }

    // Update the widgets state
    refresh();
}

//! Create a file dialog to read the setup from a file
void SetupEditor::readDialog()
{
    QString pathFile = QFileDialog::getOpenFileName(this, tr("Import Setup"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Setup file format (*.%1)").arg(fileSuffix()));
    if (pathFile.isEmpty())
        return;
    read(pathFile);
}

//! Create a file dialog to write the to a file
void SetupEditor::writeDialog()
{
    QString pathFile = QFileDialog::getSaveFileName(this, tr("Export Setup"), Utility::getLastDirectory(mSettings).path(),
                                                    tr("Setup file format (*.%1)").arg(fileSuffix()));
    if (pathFile.isEmpty())
        return;
    write(pathFile);
}

//! Helper function to build properties out of fields and their values
ResponseProperties buildProperties(QStringList const& fields, QStringList const& values)
{
    ResponseProperties result;

    // Process all the fields
    int numFields = fields.size();
    QMetaObject const& metaObject = ResponseProperties::staticMetaObject;
    for (int i = 0; i != numFields; ++i)
    {
        // Get the property
        QString field = fields[i].toLower();
        int iProperty = metaObject.indexOfProperty(field.toStdString().data());
        if (iProperty == -1)
            continue;
        QMetaProperty property = metaObject.property(iProperty);

        // Convert the value, if necessary
        QVariant value = values[i];
        if (field == "direction")
            value = (int) getDirection(value.toString());
        else if (field == "sign")
            value = value.toString() == "+" ? 1 : -1;

        // Set the property value
        property.writeOnGadget(&result, value);
    }
    return result;
}

//! Helper function to get row values from a table
QStringList rowValues(CustomTable* pTable, int iRow)
{
    int numCols = pTable->columnCount();
    QStringList result(numCols);
    for (int iCol = 0; iCol != numCols; ++iCol)
    {
        QTableWidgetItem* pItem = pTable->item(iRow, iCol);
        QString text;
        if (pItem)
            text = pItem->text();
        result[iCol] = text;
    }
    return result;
}

//! Helper function to write line to csv formatted file
void writeLine(QTextStream& stream, QChar separator, QStringList const& values)
{
    int numValues = values.size();
    for (int i = 0; i != numValues; ++i)
    {
        if (i > 0)
            stream << separator;
        stream << values[i];
    }
    stream << Qt::endl;
}
