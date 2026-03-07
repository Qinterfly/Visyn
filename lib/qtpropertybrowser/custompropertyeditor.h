#ifndef CUSTOMPROPERTYEDITOR_H
#define CUSTOMPROPERTYEDITOR_H

#include "qttreepropertybrowser.h"

class QtIntPropertyManager;
class QtDoublePropertyManager;
class QtColorPropertyManager;
class QtEnumPropertyManager;
class QtBoolPropertyManager;
class QtGroupPropertyManager;

class QtSpinBoxFactory;
class QtDoubleSpinBoxFactory;
class QtColorEditorFactory;
class QtEnumEditorFactory;
class QtCheckBoxFactory;

class CustomPropertyEditor : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    CustomPropertyEditor(QWidget* pParent = nullptr);
    virtual ~CustomPropertyEditor();

    QtProperty* createIntProperty(int id, QString const& label, int value, QtProperty* pParent = nullptr);
    QtProperty* createIntProperty(int id, QString const& label, int value, int minimum, int maximum, QtProperty* pParent = nullptr);
    QtProperty* createDoubleProperty(int id, QString const& label, double value, QtProperty* pParent = nullptr);
    QtProperty* createDoubleProperty(int id, QString const& label, double value, double minimum, double maximum, int decimals,
                                     QtProperty* pParent = nullptr);
    QtProperty* createColorProperty(int id, QString const& label, QColor const& value, QtProperty* pParent = nullptr);
    QtProperty* createEnumProperty(int id, QString const& label, int value, QStringList const& names, QtProperty* pParent = nullptr);
    QtProperty* createBoolProperty(int id, QString const& label, bool value, QtProperty* pParent = nullptr);
    QtProperty* createGroupProperty(int id, QString const& label, QtProperty* pParent = nullptr);
    bool addProperty(QtProperty* pProperty, int id, QtProperty* pParentProperty = nullptr, bool isExpanded = true);
    void clear();

    bool contains(QtProperty* pProperty) const;
    bool contains(int id) const;
    int id(QtProperty* pProperty) const;
    QtProperty* property(int id) const;

signals:
    void intValueChanged(QtProperty* pProperty, int value);
    void doubleValueChanged(QtProperty* pProperty, double value);
    void colorValueChanged(QtProperty* pProperty, QColor value);
    void boolValueChanged(QtProperty* pProperty, bool value);

private:
    void createManagers();
    void createFactories();
    bool registerProperty(QtProperty* pProperty, int id);
    bool unregisterProperty(int id);

private:
    // Mappers
    QMap<QtProperty*, int> mPropertyToID;
    QMap<int, QtProperty*> mIDToProperty;

    // Managers
    QtIntPropertyManager* mpIntManager;
    QtDoublePropertyManager* mpDoubleManager;
    QtColorPropertyManager* mpColorManager;
    QtEnumPropertyManager* mpEnumManager;
    QtBoolPropertyManager* mpBoolManager;
    QtGroupPropertyManager* mpGroupManager;

    // Factories
    QtSpinBoxFactory* mpSpinBoxFactory;
    QtDoubleSpinBoxFactory* mpDoubleSpinBoxFactory;
    QtColorEditorFactory* mpColorEditorFactory;
    QtEnumEditorFactory* mpEnumEditorFactory;
    QtCheckBoxFactory* mpCheckBoxFactory;
};

#endif // CUSTOMPROPERTYEDITOR_H
