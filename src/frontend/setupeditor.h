#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

#include <QWidget>

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
    SetupEditor(QWidget* pParent = nullptr);
    virtual ~SetupEditor();

    QSize sizeHint() const override;

    QList<Backend::Core::ResponseProperties> const& setup() const;
    void replaceProperties(QList<Backend::Core::Response>& responses) const;

    bool read(QString const& pathFile);

private:
    void createContent();

private:
    QList<Backend::Core::ResponseProperties> mSetup;
    CustomTable* mpDataTable;
};

}

#endif // SETUPEDITOR_H
