#include "modifysyncdialog.h"
#include "ui_modifysyncdialog.h"

const char* typeNames[3]={"Donwload only", "Upload only", "Download and Upload"};

ModifySyncDialog::ModifySyncDialog(QString local, QString remote,int type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifySyncDialog)
{
    ui->setupUi(this);
    ui->label_localVal->setText(local);
    ui->label_remoteVal->setText(remote);
    ui->label_DirectionVal->setText(typeNames[type -1]);
    ui->combo_Directions->setCurrentIndex(type-1);
    connect(ui->bntSave, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(hide()));
}

ModifySyncDialog::~ModifySyncDialog()
{
    delete ui;
}

int ModifySyncDialog::returnNewType()
{
    return ui->combo_Directions->currentIndex();
}
