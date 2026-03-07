#include "qteditorfactory.h"
#include "qtpropertymanager.h"
#include <QApplication>

#include "custompropertyeditor.h"

CustomPropertyEditor::CustomPropertyEditor(QWidget* pParent)
    : QtTreePropertyBrowser(pParent)
{
    setTreeWidgetFont(qApp->font());
    createManagers();
    createFactories();
}

CustomPropertyEditor::~CustomPropertyEditor()
{
    unsetFactoryForManager(mpIntManager);
    unsetFactoryForManager(mpDoubleManager);
    unsetFactoryForManager(mpColorManager);
    unsetFactoryForManager(mpEnumManager);
    unsetFactoryForManager(mpBoolManager);
    unsetFactoryForManager(mpGroupManager);

    delete mpSpinBoxFactory;
    delete mpDoubleSpinBoxFactory;
    delete mpColorEditorFactory;
    delete mpEnumEditorFactory;
    delete mpCheckBoxFactory;

    delete mpIntManager;
    delete mpDoubleManager;
    delete mpColorManager;
    delete mpEnumManager;
    delete mpBoolManager;
    delete mpGroupManager;
}

//! Create widget and register a property to edit integer values
QtProperty* CustomPropertyEditor::createIntProperty(int id, QString const& label, int value, QtProperty* pParent)
{
    return createIntProperty(id, label, value, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), pParent);
}

//! Create widget and register a property to edit integer values
QtProperty* CustomPropertyEditor::createIntProperty(int id, QString const& label, int value, int minimum, int maximum, QtProperty* pParent)
{
    QtProperty* pProperty = mpIntManager->addProperty(label);
    mpIntManager->setValue(pProperty, value);
    mpIntManager->setMinimum(pProperty, minimum);
    mpIntManager->setMaximum(pProperty, maximum);
    addProperty(pProperty, id, pParent);
    return pProperty;
}

//! Create widget and register a property to edit double values
QtProperty* CustomPropertyEditor::createDoubleProperty(int id, QString const& label, double value, QtProperty* pParent)
{
    return createDoubleProperty(id, label, value, std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 3, pParent);
}

//! Create widget and register a property to edit double values
QtProperty* CustomPropertyEditor::createDoubleProperty(int id, QString const& label, double value, double minimum, double maximum, int decimals,
                                                       QtProperty* pParent)
{
    QtProperty* pProperty = mpDoubleManager->addProperty(label);
    mpDoubleManager->setValue(pProperty, value);
    mpDoubleManager->setMinimum(pProperty, minimum);
    mpDoubleManager->setMaximum(pProperty, maximum);
    mpDoubleManager->setDecimals(pProperty, decimals);
    addProperty(pProperty, id, pParent);
    return pProperty;
}

//! Create widget and register a property to edit colors
QtProperty* CustomPropertyEditor::createColorProperty(int id, QString const& label, QColor const& value, QtProperty* pParent)
{
    QtProperty* pProperty = mpColorManager->addProperty(label);
    mpColorManager->setValue(pProperty, value);
    addProperty(pProperty, id, pParent, false);
    return pProperty;
}

//! Create widget and register a property to edit enumeration values
QtProperty* CustomPropertyEditor::createEnumProperty(int id, QString const& label, int value, QStringList const& names, QtProperty* pParent)
{
    QtProperty* pProperty = mpEnumManager->addProperty(label);
    mpEnumManager->setEnumNames(pProperty, names);
    mpEnumManager->setValue(pProperty, value);
    addProperty(pProperty, id, pParent);
    return pProperty;
}

//! Create widget and register a property to edit boolean flags
QtProperty* CustomPropertyEditor::createBoolProperty(int id, QString const& label, bool value, QtProperty* pParent)
{
    QtProperty* pProperty = mpBoolManager->addProperty(label);
    mpBoolManager->setValue(pProperty, value);
    addProperty(pProperty, id, pParent);
    return pProperty;
}

//! Create group and register a property which will be used as a parent for other properties
QtProperty* CustomPropertyEditor::createGroupProperty(int id, QString const& label, QtProperty* pParent)
{
    QtProperty* pProperty = mpGroupManager->addProperty(label);
    addProperty(pProperty, id, pParent);
    return pProperty;
}

//! Add property to the tree structure and register it
bool CustomPropertyEditor::addProperty(QtProperty* pProperty, int id, QtProperty* pParentProperty, bool isExpanded)
{
    // Try to register the property
    if (!registerProperty(pProperty, id))
        return false;

    // Insert the property and expand it
    QtBrowserItem* pItem;
    if (pParentProperty)
    {
        pParentProperty->addSubProperty(pProperty);
    }
    else
    {
        QtBrowserItem* pItem = QtTreePropertyBrowser::addProperty(pProperty);
        setExpanded(pItem, isExpanded);
    }

    return true;
}

//! Remove all the properties
void CustomPropertyEditor::clear()
{
    mPropertyToID.clear();
    mIDToProperty.clear();
    mpIntManager->clear();
    mpDoubleManager->clear();
    mpColorManager->clear();
    mpEnumManager->clear();
    mpBoolManager->clear();
    mpGroupManager->clear();
}

//! Check if the editor contains the property
bool CustomPropertyEditor::contains(QtProperty* pProperty) const
{
    return mPropertyToID.contains(pProperty);
}

//! Check if there is the property associated with the specified identifier
bool CustomPropertyEditor::contains(int id) const
{
    return mIDToProperty.contains(id);
}

//! Retrieve the property identifier
int CustomPropertyEditor::id(QtProperty* pProperty) const
{
    if (mPropertyToID.contains(pProperty))
        return mPropertyToID[pProperty];
    else
        return -1;
}

//! Retrieve the property by identifier
QtProperty* CustomPropertyEditor::property(int id) const
{
    if (mIDToProperty.contains(id))
        return mIDToProperty[id];
    else
        return nullptr;
}

//! Create the managers to handle creating different types of properties
void CustomPropertyEditor::createManagers()
{
    mpIntManager = new QtIntPropertyManager;
    mpDoubleManager = new QtDoublePropertyManager;
    mpColorManager = new QtColorPropertyManager;
    mpEnumManager = new QtEnumPropertyManager;
    mpBoolManager = new QtBoolPropertyManager;
    mpGroupManager = new QtGroupPropertyManager;

    connect(mpIntManager, &QtIntPropertyManager::valueChanged, this, &CustomPropertyEditor::intValueChanged);
    connect(mpDoubleManager, &QtDoublePropertyManager::valueChanged, this, &CustomPropertyEditor::doubleValueChanged);
    connect(mpColorManager, &QtColorPropertyManager::valueChanged, this, &CustomPropertyEditor::colorValueChanged);
    connect(mpEnumManager, &QtEnumPropertyManager::valueChanged, this, &CustomPropertyEditor::intValueChanged);
    connect(mpBoolManager, &QtBoolPropertyManager::valueChanged, this, &CustomPropertyEditor::boolValueChanged);
}

//! Construct the factories to set the delegates of the items
void CustomPropertyEditor::createFactories()
{
    mpSpinBoxFactory = new QtSpinBoxFactory;
    mpDoubleSpinBoxFactory = new QtDoubleSpinBoxFactory;
    mpColorEditorFactory = new QtColorEditorFactory;
    mpEnumEditorFactory = new QtEnumEditorFactory;
    mpCheckBoxFactory = new QtCheckBoxFactory;

    setFactoryForManager(mpIntManager, mpSpinBoxFactory);
    setFactoryForManager(mpDoubleManager, mpDoubleSpinBoxFactory);
    setFactoryForManager(mpColorManager, mpColorEditorFactory);
    setFactoryForManager(mpEnumManager, mpEnumEditorFactory);
    setFactoryForManager(mpBoolManager, mpCheckBoxFactory);
}

//! Register the property in the tree structure
bool CustomPropertyEditor::registerProperty(QtProperty* pProperty, int id)
{
    if (contains(id))
        return false;
    mPropertyToID[pProperty] = id;
    mIDToProperty[id] = pProperty;
    return true;
}

//! Unregister the property from the tree structure
bool CustomPropertyEditor::unregisterProperty(int id)
{
    if (!contains(id))
        return false;
    QtProperty* pProperty = mIDToProperty[id];
    mPropertyToID.remove(pProperty);
    mIDToProperty.remove(id);
    return true;
}
