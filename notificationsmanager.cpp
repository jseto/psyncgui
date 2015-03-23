#include "notificationsmanager.h"
#include "pcloudapp.h"

#include <QDebug>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>

NotificationsWidget::NotificationsWidget(NotificationsManager *mngr, QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    this->setWindowFlags(Qt::Dialog);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    this->setMinimumWidth(420);
    this->mngrParent = mngr;
}

/*
void NotificationsWidget::leaveEvent()
{
    qDebug()<<"NotificationsWidget::leaveEvent";
}

void NotificationsWidget::hideEvent(QHideEvent *event)
{
    qDebug()<<"hideEvent";
     bool res = this->close();
     qDebug()<<res;
     event->accept();
}

bool NotificationsWidget::eventFilter(QObject *watched, QEvent *event)
{
    qDebug()<<"eventFilter"<<watched->objectName()<<event->type() << QCursor::pos();
    if (event->type() ==QEvent::FocusIn)
        flagFocus = true;
    //if (event->type() == QEvent::WindowDeactivate)
        //this->close();
    //if (flagFocus && (event->type() ==  QEvent::Leave || event->type() ==  QEvent::FocusOut )) //9 11
    //    this->close();
    //77 repaint
}

void NotificationsWidget::mouseEvent(QMouseEvent *event)
{
    qDebug()<<"mousePressEvent";
    QPoint pos = event->globalPos();
   if (this)
   pos = this->mapFromGlobal(pos);
   if (!this->rect().contains(pos))
       this->close();
  // hideTip();
   QWidget::mouseMoveEvent(event);
}
*/

void NotificationsWidget::focusOutEvent(QFocusEvent *event)
{
    QPoint pos = QCursor::pos();
    if (this)
        pos = this->mapFromGlobal(pos);

    if (!this->rect().contains(pos))
    {
        this->close();

        if(mngrParent->getLastNtfctId() != -1)
        {
            psync_mark_notificaitons_read(mngrParent->getLastNtfctId()); //++ check errors
            mngrParent->resetNums();
        }
    }
    event->accept();
}


NotificationsManager::NotificationsManager(PCloudApp *a, QObject *parent) :
    QObject(parent)
{
    qDebug()<<"NotificationsManager create";
    numRead = 0;
    app = a;
    actnsMngrArr = NULL;
    updateFlag = false;
    lastNtfctId = -1;
    dtFontSize = app->fontPointSize -3;
    dtHtmlBeginStr = QString("<p><span style=\" font-size:").append(QString::number(dtFontSize)).append("pt; color:#797979;\">  ");
    dtHtmlEndStr = QString("</span></p>");
    table = new QTableView();
    this->setTableProps();

    notificationsModel = new QStandardItemModel(0, 2);
    table->setModel(notificationsModel);
    notifyDelegate = new NotifyDelegate(table);
    table->setItemDelegate(notifyDelegate);
    layout = new QVBoxLayout();
    hlayout = new QHBoxLayout();
    QLabel *label = new QLabel(), *icon = new QLabel();
    label->setText("pCloud Notifications");
#ifdef Q_OS_LINUX
    label->setFont(app->bigger3pFont);
#else
    label->setFont(app->bigger1pFont);
#endif
    label->setAlignment(Qt::AlignLeft);
    icon->setPixmap(QPixmap(":/48x34/images/48x34/notify.png"));
    icon->setMaximumWidth(68);
    icon->setMaximumHeight(80);
    icon->setAlignment(Qt::AlignHCenter);
    hlayout->addWidget(icon);
    hlayout->addWidget(label);
    layout->addLayout(hlayout);
    noNtfctnsLabel = new QLabel("No notifications available.");
    noNtfctnsLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    noNtfctnsLabel->setMargin(50);
    noNtfctnsLabel->setVisible(false);
    layout->addWidget(noNtfctnsLabel);
    layout->addWidget(table);
    table->setStyleSheet("QTableView{background-color:#F3F3F3;}");
    notifywin = new NotificationsWidget(this);
    notifywin->setLayout(layout);
    notifywin->show(); //temp
    notifywin->setFocus();

    connect(table, SIGNAL(clicked(QModelIndex)), notifywin, SLOT(setFocus()));
    connect(table, SIGNAL(clicked(QModelIndex)), this, SLOT(actionExecSlot(QModelIndex)));

    //connect(table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(actionExecSlot()));
}

void NotificationsManager::setTableProps()
{
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setSelectionBehavior(QTableView::SelectRows);
    table->setShowGrid(false);
    table->viewport()->setAttribute(Qt::WA_Hover);
    table->setMouseTracking(true);
    table->setMinimumHeight(400);
    QHeaderView *headerH = table->horizontalHeader(), *headerV = table->verticalHeader();
    headerH->hide();
    headerV->hide();
    table->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void NotificationsManager::loadModel(psync_notification_list_t* notifications)
{
    qDebug()<<"loadmodel"<<notifications->newnotificationcnt <<notifications->notificationcnt<<notifications->notifications;

    int ntfCnt = notifications->notificationcnt;
    notificationsModel->setRowCount(ntfCnt);
    actnsMngrArr = new actionsManager[ntfCnt];

    this->notifyDelegate->setNumNew(notifications->newnotificationcnt);
    lastNtfctId = notifications->notifications[0].notificationid;

    for (int i = 0; i < ntfCnt; i++)
    {
        qDebug()<< notifications->notifications[i].actiondata.folderid <<notifications->notifications[i].actionid <<
                   notifications->notifications[i].iconid<<notifications->notifications[i].isnew<<
                   notifications->notifications[i].mtime << notifications->notifications[i].notificationid
                <<notifications->notifications[i].text <<notifications->notifications[i].thumb;

        QModelIndex indexHtml = notificationsModel->index(i, 1, QModelIndex());
        QString htmldata(notifications->notifications[i].text);
        htmldata.append(dtHtmlBeginStr).append((QDateTime::fromTime_t(notifications->notifications[i].mtime).toString())).append(dtHtmlEndStr);
        notificationsModel->setData(indexHtml, QVariant(htmldata), Qt::EditRole); //displayrole

        actnsMngrArr[i].actionId = notifications->notifications[i].actionid;
        if(actnsMngrArr[i].actionId)
            actnsMngrArr[i].actionData = notifications->notifications[i].actiondata;
        //        qDebug()<<"actnsMngrArr"<<actnsMngrArr[i].actionId<<actnsMngrArr[i].actionData.folderid;

        QModelIndex indexIcon = notificationsModel->index(i, 0, QModelIndex());
        QString iconpath = "/home/damyanka/git/psyncguiSeptm/psyncgui/images/testNtf.png";  //notifications->notifications[i].iconid
        notificationsModel->setData(indexIcon, QVariant(iconpath));

        table->openPersistentEditor(notificationsModel->index(i, 1));
        table->openPersistentEditor(notificationsModel->index(i, 0));

        table->resizeRowsToContents();
    }
}

void NotificationsManager::clearModel()
{
    notificationsModel->removeRows(0, notificationsModel->rowCount());

    if (actnsMngrArr != NULL)
    {
        free(actnsMngrArr);
        actnsMngrArr = NULL;
    }
    this->resetNums();
}

void NotificationsManager::init()
{
    psync_notification_list_t* notifications = psync_get_notifications();
    if(notifications != NULL && notifications->notificationcnt)
    {
        this->loadModel(notifications);
        free(notifications);
        notifications = NULL;
    }
    else
    {
        qDebug()<<"initmodel no notif";
        this->table->setVisible(false);
        this->noNtfctnsLabel->setVisible(true);
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    /*
     * OLD INIT TESTING MODEL
    notificationsModel->setRowCount(10);
    for (int row = 0; row < notificationsModel->rowCount()-1; ++row)
    {
        {
            QModelIndex index = notificationsModel->index(row, 1, QModelIndex());
            //QString value = "<html><head/><body> <ul> <li>Secure place for private/business use</li> <li>Zero-Knowledge Privacy</li> </ul></body></html>"; //"test<b>fdsfdsfs</b>";
            QString value;
            if (row%2)
                value = "<html><head/><body><p><span style=\" font-weight:600;\">User dlfdlsfjlsfjlsfjlsfjlsfjdlssadjkas@afd.bg<br>edited file blqbql<br>in folder lqlq2</span></p></body></html>"
                        "      <p><span style=\" font-size:9pt; color:#2bc1d1;\">   dateexm</span></p></body></html>";
            else
                //  value = "<html><head/><body><p><span style=\" font-weight:600;\">User dlsadjcdfsfsfdfgskas@afd.bg<br>edited file blqbql<br>in folder lqlq2</span></p></body></html>"
                //        "<p><span style=\" font-size:9pt; color:#2bc1d1;\">   dateexm<br>нов ред<br>another one</span></p></body></html>";
                value = "We must be <b>bold</b>, very <b>bold</b><br><p><span style=\" font-size:9pt; color:#2bc1d1;\">   dateexm</span></p>";

            notificationsModel->setData(index, QVariant(value),Qt::EditRole);
            notificationsModel->setData(index,QVariant("datatest"), Qt::UserRole+1);
            QModelIndex indexIcon = notificationsModel->index(row, 0, QModelIndex());
            //QString iconpath = "/home/damyanka/git/psyncguiSeptm/psyncgui/images/menu 48x48/info.png";
            QString iconpath = "/home/damyanka/git/psyncguiSeptm/psyncgui/images/testNtf.png";
            notificationsModel->setData(indexIcon, QVariant(iconpath));
        }
    }


    for (int i = 0; i < notificationsModel->rowCount(); ++i)
    {
        table->openPersistentEditor(notificationsModel->index(i, 1)) ;
        table->openPersistentEditor(notificationsModel->index(i, 0)) ;
    }
    */
}

void NotificationsManager::clear()
{
    this->clearModel();
}

void NotificationsManager::updateNotfctnsModel(int newcnt)
{
    // if(!notifywin->isVisible())
    //  if(this->lastNtfctId != -1 && newcnt) // callbacked is called after marking red
    if(newcnt) // callbacked is called after marking red
    {
        psync_notification_list_t* notifications = psync_get_notifications();

        //numread = new
        qDebug()<<"updateNotfctnsModel"<< notifications->newnotificationcnt << notifications->notificationcnt;
        if (notifications != NULL && notifications->newnotificationcnt) //first notifications
        {
            if(!table->isVisible())
            {
                noNtfctnsLabel->setVisible(false);
                table->setVisible(true);
            }

            this->clearModel();
            this->loadModel(notifications);

            /*     for (int i = 0; i < cnt; i++)
        {
            qDebug()<< notifications->notifications[i].actiondata.folderid <<notifications->notifications[i].actionid <<
                       notifications->notifications[i].iconid<<notifications->notifications[i].isnew<<
                       notifications->notifications[i].mtime << notifications->notifications[i].notificationid
                    <<notifications->notifications[i].text <<notifications->notifications[i].thumb;

            QModelIndex indexHtml = notificationsModel->index(i, 1, QModelIndex());
            //QString htmldata("<b>");
            QString htmldata(notifications->notifications[i].text); //    htmldata.append(notifications->notifications[i].text);
            //      htmldata.append("</b>");
            // htmldata.append("<p><span style=\" font-size:").append(QString::number(dtFontSize)).append("pt;\">").append((QDateTime::fromTime_t(notifications->notifications[i].mtime).toString())).append("</span></p>");
            htmldata.append(dtHtmlBeginStr).append((QDateTime::fromTime_t(notifications->notifications[i].mtime).toString())).append(dtHtmlEndStr);

            notificationsModel->setData(indexHtml, QVariant(htmldata), Qt::EditRole); //displayrole

            actionsManager actionInfo; //(notifications->notifications[i].actionid,notifications->notifications[i].actiondata);
            if(notifications->notifications[i].actionid)
            {
                actionInfo.actionData = notifications->notifications[i].actiondata;
                actionInfo.actionId = notifications->notifications[i].actionid;
                //actnsMngrArr[i].actionId = notifications->notifications[i].actionid;
                //notificationsModel->setData(indexhtml, QVariant(notifications->notifications[i].actionid), (Qt::UserRole+1));

            }
            else
                actionInfo.actionId= 0;

            actnsMngrArr[i] = actionInfo;

            qDebug()<<"actnsMngrArr"<<actnsMngrArr[i].actionId<<actnsMngrArr[i].actionData.folderid;

            QModelIndex indexIcon = notificationsModel->index(i, 0, QModelIndex());
            QString iconpath = "/home/damyanka/git/psyncguiSeptm/psyncgui/images/testNtf.png";  //notifications->notifications[i].iconid
            notificationsModel->setData(indexIcon, QVariant(iconpath));

            table->openPersistentEditor(notificationsModel->index(i, 1)) ;
            table->openPersistentEditor(notificationsModel->index(i, 0)) ;
        }
*/
            free(notifications);
            notifications = NULL;

            //emit chage tray
        }
        else
            qDebug()<<"updateNotfctnsModel no new notifications";
    }
    //else
    //  updateFlag = true;

}

void NotificationsManager::showNotificationsWin()
{
    notifywin->show();
    notifywin->setFocus();

    //if !num - set dflt text, hide table
    //notifywin->raise();
    //notifywin->activateWindow();
    //notifywin->showNormal();
    //notifywin->setWindowState(Qt::WindowActive);
    //a.setActiveWindow(notifywin);
}

void NotificationsManager::resetNums()
{
    notifyDelegate->setNumNew(0);
    this->lastNtfctId = -1;
}

void NotificationsManager::actionExecSlot(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    switch(actnsMngrArr[index.row()].actionId)
    {
    case PNOTIFICATION_ACTION_NONE: //0
        return;
    case PNOTIFICATION_ACTION_GO_TO_FOLDER: //1
    {
        char *path = psync_fs_get_path_by_folderid(actnsMngrArr[index.row()].actionData.folderid);
        if(path != NULL)
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            free(path);
        }
        break;
    }
    default:
        break;
    }
}

quint32 NotificationsManager::getLastNtfctId()
{
    return this->lastNtfctId;
}