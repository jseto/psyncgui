#ifndef CRYPTOPAGE_H
#define CRYPTOPAGE_H
#include "pcloudwindow.h"
#include "pcloudapp.h"
#include "cryptokeydialog.h"

#include <QObject>

class PCloudWindow;

class CryptoPage : public QObject
{
    Q_OBJECT
    friend class CryptoKeyDialog;
public:
    explicit CryptoPage(PCloudWindow *w, PCloudApp *a,QObject *parent = 0);
    void showEventCrypto();
    void initCryptoPage();
    int getCurrentCryptoPageIndex();
private:
    PCloudApp* app;
    PCloudWindow* win;
    bool tryTrialClickedFlag;
    int pageIndex, passStrenth;
    void setCurrentPageIndex();
    void setTrialUI(bool hasSubscriptoin, uint expTime);
    void showSetupCryptoError(int setupRes);
    void showStartCryptoError(int startRes);
    void showMkDirWrror(int mkdirRes, const char* err);
    void showStopCryptoError(int stopRes);
public slots:
    void lock();
    void unlock();
    void openCryptoFldr();
private slots:
    void changePage();
    void tryTrial();
    void buyCrypto();
    void getMoreCryptoInfo();
    void setupCrypto();
    void requestCryptoKey();
    void manageCryptoFldr();
    void setUnlockedFldrUI();
    void setLockedFldrUI();    
    void setProgressBar();
    void checkPasswordsMatch();
};

#endif // CRYPTOPAGE_H
