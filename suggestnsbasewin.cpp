#include "suggestnsbasewin.h"
#include "ui_suggestnsbasewin.h"
#include "addsyncdialog.h"
#include "modifysyncdialog.h"
#include "ui_modifysyncdialog.h"
#include "psynclib.h"
#include "common.h"

#include <QDebug>
#include <QComboBox>
#include <QStandardItemModel>

//this class is used from windows context menu for adding many folders at the same time for sync

const char* typeStr[3]={"Donwload only", "Upload only", "Download and Upload"};

SuggestnsBaseWin::SuggestnsBaseWin(PCloudApp *a, QStringList *fldrs, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::SuggestnsBaseWin)
{
    app = a;
    ui->setupUi(this);
    isChangingItem = false;
    localFldrsLst = fldrs;

    connect(ui->btnAdd, SIGNAL(clicked()),this, SLOT(addSync()));
    connect(ui->btnFinish, SIGNAL(clicked()), this, SLOT(finish()));
    connect(ui->treeWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(changeCurrItem(QModelIndex)));
    connect(ui->btnModify, SIGNAL(clicked()), this, SLOT(modifyType()));
    ui->statusbar->setVisible(false);

    this->setWindowTitle(trUtf8("pCloud Sync"));
    this->setWindowIcon(QIcon(WINDOW_ICON));

    //default synced fldrs
    QString root = "/";
    pfolder_list_t *remoteRootChildFldrs = psync_list_remote_folder_by_path(root.toUtf8(),PLIST_FOLDERS);
    //build list with remote root child names in order to check: when we add new sync delay if the folderName exists.
    for (uint i = 0; i < remoteRootChildFldrs->entrycnt; i++)
        remoteFldrsNamesLst.append(remoteRootChildFldrs->entries[i].name);
    qDebug()<<"Remote init folders list form lib"<<remoteFldrsNamesLst;

    /*
    QString defaultRemoteFldr = "/pCloudSync";

    QTreeWidgetItem *defaultItem = new QTreeWidgetItem(ui->treeWidget); // the default sync; the first item in the view; uneditable
    defaultItem->setCheckState(0,Qt::Checked);
    defaultItem->setFlags(Qt::NoItemFlags);

    QString path = QDir::home().path().append("/pCloudSync");
    QDir pcloudDir(path);
    QList<QStringList> itemsLst;
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
    QDir docs (QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
    QDir music(QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    QDir photos(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    QDir movies(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
#else
    QDir docs(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    QDir music(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    QDir photos(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    QDir movies(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
#endif

    QString nativepath;
#ifdef Q_OS_WIN
    QString docspath;
    nativepath = pcloudDir.toNativeSeparators(path);
    docspath = docs.toNativeSeparators(docs.absolutePath());
    itemsLst << (QStringList() << docspath << "/My Documents");
    // in win xp My music, My pictures and My videos are located in My documents by default
    if (QSysInfo::windowsVersion() != QSysInfo::WV_XP)
    {
        QString musicpath,moviespath, photospath;
        musicpath = music.toNativeSeparators(music.absolutePath());
        moviespath = movies.toNativeSeparators(music.absolutePath());
        photospath = photos.toNativeSeparators(photos.absolutePath());
        itemsLst << (QStringList() << musicpath << "/My Music")
                 << (QStringList() << photospath << "/My Pictures")
                 << (QStringList() << moviespath << "/My Videos");
    }
#else
    nativepath = pcloudDir.path();
    itemsLst << (QStringList() << docs.absolutePath() << "/My Documents")
             << (QStringList() << music.absolutePath() << "/My Music")
             << (QStringList() << photos.absolutePath() << "/My Pictures")
             << (QStringList() << movies.absolutePath() << "/My Videos");
#endif
    if(!pcloudDir.exists())
    {
        QDir::home().mkdir("pCloudSync");
        remoteFldrsNamesLst.append("pCloudSync");
        newRemoteFldrsLst.append("pCloudSync");
    }

    defaultItem->setText(1,nativepath);
    defaultItem->setData(1,Qt::UserRole, nativepath);
    defaultItem->setText(2,trUtf8("Download and Upload"));
    defaultItem->setData(2,Qt::UserRole, PSYNC_FULL -1); //for combos and typestr[] indexes
    defaultItem->setText(3,defaultRemoteFldr);
    defaultItem->setData(3,Qt::UserRole,defaultRemoteFldr);
    ui->treeWidget->insertTopLevelItem(0,defaultItem);

    //remoteFldrsNamesLst.append("My Documents");
    //newRemoteFldrsLst.append("My Documents");

    for(int i = 0; i < itemsLst.length(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setCheckState(0,Qt::Unchecked);
        item->setData(0,Qt::UserRole,false);
        item->setText(1,itemsLst.at(i).at(0));
        item->setData(1, Qt::UserRole,itemsLst.at(i).at(0));
        item->setText(3, itemsLst.at(i).at(1));
        item->setData(3, Qt::UserRole, itemsLst.at(i).at(1));
        item->setText(2, typeStr[2]);
        item->setData(2, Qt::UserRole, PSYNC_FULL - 1);
    }
*/
    //ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->setColumnWidth(0,40);
    ui->treeWidget->setColumnWidth(1,200);
    ui->treeWidget->setColumnWidth(2,140);
    ui->treeWidget->resizeColumnToContents(3);
    ui->treeWidget->setMinimumWidth(400);
    //ui->tableWidget->setItemDelegate(new SyncItemsDelegate());
    // this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    //if(localFldrsLst->size())
    if(localFldrsLst != NULL)
    {
        addLocalFldrs(localFldrsLst);
    }
}

void SuggestnsBaseWin::addLocalFldrs(QStringList *itemsLst)
{           
    if(ui->treeWidget->topLevelItemCount())
        ui->treeWidget->clear();
    for(int i = 0; i < itemsLst->size(); i++)
    {
        qDebug()<<"Qt: fill suggestions tree"<<i<<itemsLst->at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setCheckState(0,Qt::Checked);
        item->setData(0,Qt::UserRole,true);
        item->setText(1,itemsLst->at(i)); //localpath
        item->setData(1, Qt::UserRole,itemsLst->at(i));
        item->setText(2, typeStr[2]); //synctype
        item->setData(2, Qt::UserRole, PSYNC_FULL - 1);
#ifdef Q_OS_WIN
        QString name = itemsLst->at(i).section("\\",-1);
#else
        QString name = itemsLst->at(i).section("/",-1);
#endif
        QString remoteFldrName = checkRemoteName(name); // rename if remote folder with the same name exists
        newRemoteFldrsLst.append(remoteFldrName);
        remoteFldrsNamesLst.append(remoteFldrName);
        remoteFldrName.insert(0,"/");        
        item->setText(3,remoteFldrName); //remotepath
        item->setData(3,Qt::UserRole,remoteFldrName);
    }
}

SuggestnsBaseWin::~SuggestnsBaseWin()
{
    delete ui;
}
void SuggestnsBaseWin::closeEvent(QCloseEvent *event)
{  
    this->hide();
    event->ignore();
}
void SuggestnsBaseWin::addSync()
{
    addSyncDialog *addDialog = new addSyncDialog(app,app->pCloudWin, app->pCloudWin->get_sync_page(),this);
    if (this->isChangingItem)
    {
        currentLocal = ui->treeWidget->currentItem()->data(1,Qt::UserRole).toString();
        currentType = ui->treeWidget->currentItem()->data(2,Qt::UserRole).toInt();
        currentRemote = ui->treeWidget->currentItem()->data(3,Qt::UserRole).toString();
    }
    addDialog->exec();

}
//when user double-clicks on an item
void SuggestnsBaseWin::changeCurrItem(QModelIndex index)
{
    // if(index.row()) // the first item in the treeview is default and shoudn't be modified

    this->isChangingItem = true;
    emit this->addSync();

}
bool SuggestnsBaseWin::getChangeItem()
{
    return this->isChangingItem;
}

void SuggestnsBaseWin::addNewItem(QString &localpath, QString &remotepath, int type)
{
    QTreeWidgetItem *item;
    if(isChangingItem)
    {
        isChangingItem = false;
        item = ui->treeWidget->currentItem();
    }
    else
        item = new QTreeWidgetItem(ui->treeWidget);

    item->setCheckState(0,Qt::Checked);
    item->setData(0,Qt::UserRole,true);
    item->setText(1,localpath);
    item->setData(1,Qt::UserRole,localpath);
    item->setText(3,remotepath);
    item->setData(3,Qt::UserRole,remotepath); //da e remote path
    item->setText(2,typeStr[type]);
    item->setData(2,Qt::UserRole,type);
}

void SuggestnsBaseWin::modifyType()
{
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if(!current)
    {
        QMessageBox::information(this,"",trUtf8("Please select an item to modify"));
        return;
    }
    else
    {
        ModifySyncDialog dialog(current->data(1,Qt::UserRole).toString(),current->data(3,Qt::UserRole).toString(), current->data(2,Qt::UserRole).toInt());
        if (dialog.exec() == QDialog::Accepted)
        {
            int newType = dialog.returnNewType();
            current->setData(2,Qt::UserRole, newType);
            dialog.hide();
            current->setText(2,typeStr[newType]);
            // ++ modify sync type // type++
        }
    }
}
void SuggestnsBaseWin::finish()
{
    QStringList fldrActionsLst;
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
        qDebug()<<"SuggestnsBaseWin::finish";
        if ((*it)->checkState(0) == Qt::Checked)
        {
            QString localpath = (*it)->data(1,Qt::UserRole).toString(); //to del
            qDebug()<<"ADDING item (from suggestionswin) " << (*it)->data(1,Qt::UserRole).toString().toUtf8() << (*it)->data(3,Qt::UserRole).toString().toUtf8() <<((*it)->data(2,Qt::UserRole).toInt() +1);

            //welcome win case and contex menu case, because it's possible remote folder not to exists
            psync_add_sync_by_path_delayed(//localpath.toUtf8(),
                                           (*it)->data(1,Qt::UserRole).toString().toUtf8(),
                                           (*it)->data(3,Qt::UserRole).toString().toUtf8(),
                                           ((*it)->data(2,Qt::UserRole).toInt()) +1);

#ifdef Q_OS_WIN
            QString name = localpath.section("\\", -1);
#else
            QString name = localpath.section("/", -1);
#endif
            qDebug() << name;
            if (!fldrActionsLst.contains(name))
            {
                QAction *fldrAction = new QAction(QIcon(":/menu/images/menu 48x48/emptyfolder.png"),name, app);
                fldrAction->setProperty("path", localpath);
                connect(fldrAction, SIGNAL(triggered()), app, SLOT(openLocalDir()));
                app->addNewFolderInMenu(fldrAction);
                fldrActionsLst<<name;
            }
        }
        ++it;
    }
    app->pCloudWin->get_sync_page()->load();
    //refresh menu
    this->hide();
    if(app->isFirstLaunch)
        app->showSync(); // if first launch
}
QString SuggestnsBaseWin::checkRemoteName(QString &entryName)
{
    if (remoteFldrsNamesLst.contains(entryName))
    {
        int i = 1;
        QString newName = entryName;
        while (remoteFldrsNamesLst.contains(newName)) {
            newName = entryName + "(" + QString::number(i) + ")";
            i++;
        }
        return newName;
    }
    else
        return entryName;
}
void SuggestnsBaseWin::addNewRemoteFldr(QString &name)
{
    this->remoteFldrsNamesLst.append(name);
}

QString SuggestnsBaseWin::getCurrLocalPath()
{
    return this->currentLocal;
}
QString SuggestnsBaseWin::getCurrRemotePath()
{
    return this->currentRemote;
}
int SuggestnsBaseWin::getCurrType()
{
    return this->currentType;
}
QTreeWidgetItem* SuggestnsBaseWin::getCurrItem()
{
    return ui->treeWidget->currentItem();
}
void SuggestnsBaseWin::setChangeItem(bool b)
{
    this->isChangingItem = b;
}
