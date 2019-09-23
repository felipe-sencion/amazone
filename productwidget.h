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

private:
    Ui::ProductWidget *ui;
};

#endif // PRODUCTWIDGET_H
