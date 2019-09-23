#include "productwidget.h"
#include "ui_productwidget.h"
#include <QPixmap>

ProductWidget::ProductWidget(QString imageFile, QString description, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductWidget)
{
    ui->setupUi(this);
    ui->imageLabel->setPixmap(QPixmap(":/imgs/" + imageFile + ".jpg").scaled(160, 160, Qt::KeepAspectRatio));
    ui->productLabel->setText(description);
}

ProductWidget::~ProductWidget()
{
    delete ui;
}
