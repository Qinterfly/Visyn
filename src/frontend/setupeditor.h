#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

#include <QWidget>

#include "customlineedit.h"
#include "uialiasdata.h"

QT_FORWARD_DECLARE_CLASS(QSettings)

namespace Backend::Core
{
struct ResponseProperties;
class Response;
}

namespace Frontend
{

class CustomTable;

class SetupEditor : public QWidget
{
    Q_OBJECT

public:
    SetupEditor(QSettings& settings, QWidget* pParent = nullptr);
    virtual ~SetupEditor();

    QSize sizeHint() const override;
    QString fileSuffix() const;
    QList<Backend::Core::ResponseProperties> const& setup() const;

    void replaceProperties(QList<Backend::Core::Response>& responses) const;
    bool read(QString const& pathFile);
    bool write(QString const& pathFile);

private:
    void refresh();
    void createContent();

    // Slots
    void setNumData();
    void setData();
    void readDialog();
    void writeDialog();

private:
    QSettings& mSettings;
    QList<Backend::Core::ResponseProperties> mSetup;

    // Widgets
    CustomTable* mpDataTable;
    Edit1i* mpNumDataEdit;
};

}

#endif // SETUPEDITOR_H
