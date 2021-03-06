#ifndef REMOTETREESDIALOG_H
#define REMOTETREESDIALOG_H

#include "psynclib.h"
#include "common.h"
#include <QDialog>
#include <QShowEvent>
#include <QTreeWidgetItem>
#include <QMessageBox>

class PCloudWindow;

namespace Ui {
class RemoteTreesDialog;
}

class RemoteTreesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteTreesDialog(QString curritem, QWidget *parent);
    ~RemoteTreesDialog();
    quint64 getFldrid();       
    QString getFldrPath();
    void init();
protected:
    Ui::RemoteTreesDialog *ui;   
    quint64 fldrid;
    QString fldrPath;
    void showEvent(QShowEvent *event); 
private:
    QTreeWidgetItem* root;
    QString currentItemPath;
public slots:
    QString newRemoteFldr();
    virtual void setSelectedFolder();
};

#endif // REMOTETREESDIALOG_H
