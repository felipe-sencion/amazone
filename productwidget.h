#ifndef PRODUCTWIDGET_H
#define PRODUCTWIDGET_H

#include <QWidget>

namespace Ui {
class ProductWidget;
}

class ProductWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProductWidget(QString imageFile, QString description, QWidget *parent = 0);
    ~ProductWidget();

    QString getDescription() const;
    void setDescription(const QString &value);

    QString getId() const;
    void setId(const QString &value);

    int getSold() const;
    void setSold(int value);

private slots:
    void on_addPB_clicked();

    void on_amountSB_valueChanged(int arg1);

signals:
    void added(int amount);

private:
    Ui::ProductWidget *ui;
    QString description;
    QString id;
    int sold;
};

#endif // PRODUCTWIDGET_H
