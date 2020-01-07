#include "productwidget.h"
#include "ui_productwidget.h"
#include <QPixmap>
#include <QDebug>

ProductWidget::ProductWidget(QString imageFile, QString description, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductWidget)
{
    ui->setupUi(this);
    ui->imageLabel->setPixmap(QPixmap(":/imgs/" + imageFile + ".jpg").scaled(120, 120, Qt::KeepAspectRatio));
    ui->productLabel->setText(description);
    //id = imageFile;
    //this->description = description;
}

ProductWidget::~ProductWidget()
{
    delete ui;
}

void ProductWidget::on_addPB_clicked()
{
    qDebug()<<"click en " <<description;
    emit added(ui->amountSB->value());
}

int ProductWidget::getSold() const
{
    return sold;
}

void ProductWidget::setSold(int value)
{
    sold = value;
}

QString ProductWidget::getId() const
{
    return id;
}

void ProductWidget::setId(const QString &value)
{
    id = value;
}

QString ProductWidget::getDescription() const
{
    return description;
}

void ProductWidget::setDescription(const QString &value)
{
    description = value;
}

void ProductWidget::on_amountSB_valueChanged(int arg1)
{
    if (arg1 > 0)
        ui->addPB->setEnabled(true);
    else
        ui->addPB->setEnabled(false);
}
