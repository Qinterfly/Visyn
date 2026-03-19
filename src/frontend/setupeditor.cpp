#include <QVBoxLayout>

#include "customtable.h"
#include "fileutility.h"
#include "response.h"
#include "setupeditor.h"

using namespace Backend;
using namespace Backend::Core;
using namespace Frontend;

ResponseProperties buildProperties(QStringList const& fields, QStringList const& values);

SetupEditor::SetupEditor(QWidget* pParent)
    : QWidget(pParent)
{
    setWindowTitle(tr("Setup"));
    createContent();
}

SetupEditor::~SetupEditor()
{
}

QSize SetupEditor::sizeHint() const
{
    return QSize(600, 300);
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
            QString prefix = response.isComplex() ? "Harmonic Spectrum" : "Time Signal";
            props.name = QString("%1 %2:%3:%4%5").arg(prefix, props.component, props.node, sign, getLabel(props.direction));
        }
    }
}

//! Read response props from a file
bool SetupEditor::read(QString const& pathFile)
{
    // Constants
    QString const kFileSuffix = "csv";

    // Clear the previous properties
    mSetup.clear();

    // Open the file for reading
    auto pFile = Utility::openFile(pathFile, kFileSuffix, QIODevice::ReadOnly);
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
    for (QString& f : fields)
        f = f.toLower();

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

    return true;
}

//! Create all the widgets and corresponding content
void SetupEditor::createContent()
{
    QVBoxLayout* pLayout = new QVBoxLayout;
    mpDataTable = new CustomTable;
    pLayout->addWidget(mpDataTable);
    setLayout(pLayout);
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
        QString field = fields[i];
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
