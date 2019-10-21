#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QScrollArea>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileMenu = ui->menuBar->addMenu(tr("&Archivo"));

    openFileAction = new QAction(tr("Abrir archivo"), this);
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
    fileMenu->addAction(openFileAction);
}

MainWindow::~MainWindow()
{
    delete ui;
    saveDB();
}

void MainWindow::enableLoginPB()
{
    if (ui->emailLE->text().length() > 0 && ui->passwordLE->text().length() > 0)
        ui->loginPB->setEnabled(true);
    else
        ui->loginPB->setEnabled(false);
}

void MainWindow::enableCreatePB()
{
    if (ui->newNameLE->text().length() > 0
            && ui->newMailLE->text().length() > 0
            && ui->newPasswordLE->text().length() > 0)
        ui->createPB->setEnabled(true);
    else
        ui->createPB->setEnabled(false);
}

void MainWindow::loadDB()
{
    dbFile.open(QIODevice::ReadOnly);
    QByteArray data = dbFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument(QJsonDocument::fromJson(data));
    QJsonObject jsonObj = jsonDoc.object();
    jsonDB = jsonObj["users"].toArray();
    products = jsonObj["products"].toArray();
    productsCopy = products;
    for (int i(0); i < jsonDB.size(); ++i)
    {
        User u;
        QJsonObject o = jsonDB[i].toObject();
        u.setEmail(o["email"].toString());
        u.setName(o["name"].toString());
        u.setPassword(o["password"].toString());
        users.push_back(u);
    }
    for (int i(0); i < products.size(); ++i)
    {
        QJsonObject o = products[i].toObject();
        ProductWidget *p;
        p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
        ui->auxGrid->addWidget(p, i/3, i%3, Qt::AlignCenter);
    }
    dbFile.close();
}

void MainWindow::saveDB()
{
    dbFile.open(QIODevice::WriteOnly);
    QJsonObject jsonObj;
    QJsonDocument jsonDoc;
    jsonObj["users"] = jsonDB;
    jsonObj["products"] = products;
    jsonDoc = QJsonDocument(jsonObj);

    dbFile.write(jsonDoc.toJson());
    dbFile.close();
}

void MainWindow::clearProductsArea()
{
    QLayoutItem* item;
    while( (item = ui->auxGrid->takeAt(0)) )
    {
        delete item->widget();
        delete item;
    }
}

void MainWindow::on_emailLE_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    enableLoginPB();
}

void MainWindow::on_passwordLE_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    enableLoginPB();
}

void MainWindow::on_loginPB_clicked()
{
    QMessageBox message;
    size_t i = 0;
    for (; i< users.size(); ++i)
    {
        if (users.at(i).getEmail() == ui->emailLE->text())
        {
            if (users.at(i).getPassword() == ui->passwordLE->text())
            {
                ui->amazoneSW->setCurrentIndex(2);
                this->setMinimumSize(1000, 600);
                break;
            }
            else
            {
                message.setText("La contraseña no coincide");
                message.setIcon(QMessageBox::Warning);
                message.exec();
                break;
            }
        }
    }
    if (i == users.size())
    {
        message.setText("El usuario no existe");
        message.setIcon(QMessageBox::Warning);
        message.exec();
    }
    ui->emailLE->clear();
    ui->passwordLE->clear();
}

void MainWindow::on_newUserPB_clicked()
{
    ui->amazoneSW->setCurrentIndex(1);
}

void MainWindow::on_newNameLE_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    enableCreatePB();
}

void MainWindow::on_newMailLE_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    enableCreatePB();
}

void MainWindow::on_newPasswordLE_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);

    enableCreatePB();
}

void MainWindow::on_createPB_clicked()
{
    QMessageBox message;
    size_t i = 0;
    User u;
    unsigned int idx;

    for (; i < users.size(); ++i)
    {
        if (users.at(i).getEmail() == ui->newMailLE->text())
        {
            message.setText("El usuario ya existe");
            message.setIcon(QMessageBox::Warning);
            idx = 1;
            break;
        }
    }
    if (i == users.size())
    {
        QJsonObject jsonObj;
        u.setName(ui->newNameLE->text());
        u.setEmail(ui->newMailLE->text());
        u.setPassword(ui->newPasswordLE->text());
        users.push_back(u);

        jsonObj["name"] = ui->newNameLE->text();
        jsonObj["email"] = ui->newMailLE->text();
        jsonObj["password"] = ui->newPasswordLE->text();
        jsonDB.append(jsonObj);
        message.setText("Usuario creado con éxito");
        message.setIcon(QMessageBox::Information);
        idx = 0;
    }
    message.exec();
    ui->newNameLE->clear();
    ui->newMailLE->clear();
    ui->newPasswordLE->clear();
    ui->amazoneSW->setCurrentIndex(idx);
}

void MainWindow::openFile()
{
    fileName = QFileDialog::getOpenFileName(this, "Amazone DB", "", "archivos JSON (*.json)");
    if (fileName != "")
    {
        dbFile.setFileName(fileName);
        loadDB();
    }
}

void MainWindow::on_departmentDB_currentIndexChanged(const QString &arg1)
{
    int counter = 0;
    char pattern;

    ui->searchLE->clear();
    clearProductsArea();
    if (ui->departmentDB->currentIndex() == 0)
    {
        for (int i(0); i < productsCopy.size(); ++i)
        {
            QJsonObject o = productsCopy[i].toObject();
            ProductWidget *p;

            p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
            ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
            ++counter;
        }
    }
    else
    {
        if (arg1 == "Alimentos y Bebidas")
            pattern = 'A';
        else if (arg1 == "Libros")
            pattern = 'L';
        else if (arg1 == "Electrónicos")
            pattern = 'E';
        else if (arg1 == "Hogar y Cocina")
            pattern = 'H';
        else
            pattern = 'D';

        for (int i(0); i < productsCopy.size(); ++i)
        {
            QJsonObject o = productsCopy[i].toObject();
            if (o["id"].toString()[0] == pattern)
            {
                ProductWidget *p;
                p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                ++counter;
            }
        }
    }
}

bool leq(QJsonValue o1, QJsonValue o2)
{
    return o1.toObject()["price"].toDouble() < o2.toObject()["price"].toDouble();
}

bool geq(QJsonValue o1, QJsonValue o2)
{
    return o1.toObject()["price"].toDouble() > o2.toObject()["price"].toDouble();
}

inline void swap(QJsonValueRef v1, QJsonValueRef v2)
{
    QJsonValue temp(v1);
    v1 = QJsonValue(v2);
    v2 = temp;
}

void MainWindow::on_searchLE_textChanged(const QString &arg1)
{
    int counter = 0;
    char pattern;

    clearProductsArea();
    if (ui->departmentDB->currentIndex() == 0)
    {
        for (int i(0); i < productsCopy.size(); ++i)
        {
            QJsonObject o = productsCopy[i].toObject();

            if (o["name"].toString().contains(arg1, Qt::CaseInsensitive))
            {
                ProductWidget *p;
                p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                ++counter;
            }
        }
    }
    else
    {
        if (ui->departmentDB->currentText() == "Alimentos y Bebidas")
            pattern = 'A';
        else if (ui->departmentDB->currentText() == "Libros")
            pattern = 'L';
        else if (ui->departmentDB->currentText() == "Electrónicos")
            pattern = 'E';
        else if (ui->departmentDB->currentText() == "Hogar y Cocina")
            pattern = 'H';
        else
            pattern = 'D';

        for (int i(0); i < productsCopy.size(); ++i)
        {
            QJsonObject o = productsCopy[i].toObject();
            if (o["id"].toString()[0] == pattern && o["name"].toString().contains(arg1, Qt::CaseInsensitive))
            {
                ProductWidget *p;
                p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                ++counter;
            }
        }
    }
}

void MainWindow::on_sortCB_currentIndexChanged(int index)
{
    int counter = 0;
    char pattern;
    clearProductsArea();
    if (index == 1)
        sort(productsCopy.begin(), productsCopy.end(), geq);
    else if (index == 2)
        sort(productsCopy.begin(), productsCopy.end(), leq);

    if (ui->searchLE->text().length() == 0)
    {
        if (ui->departmentDB->currentIndex() == 0)
        {
            for (int i(0); i < productsCopy.size(); ++i)
            {
                QJsonObject o = productsCopy[i].toObject();
                ProductWidget *p;

                p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                ++counter;
            }
        }
        else
        {
            if (ui->departmentDB->currentText() == "Alimentos y Bebidas")
                pattern = 'A';
            else if (ui->departmentDB->currentText() == "Libros")
                pattern = 'L';
            else if (ui->departmentDB->currentText() == "Electrónicos")
                pattern = 'E';
            else if (ui->departmentDB->currentText() == "Hogar y Cocina")
                pattern = 'H';
            else
                pattern = 'D';

            for (int i(0); i < productsCopy.size(); ++i)
            {
                QJsonObject o = productsCopy[i].toObject();
                if (o["id"].toString()[0] == pattern)
                {
                    ProductWidget *p;
                    p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                    ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                    ++counter;
                }
            }
        }
    }
    else
    {
        if (ui->departmentDB->currentIndex() == 0)
        {
            for (int i(0); i < productsCopy.size(); ++i)
            {
                QJsonObject o = productsCopy[i].toObject();

                if (o["name"].toString().contains(ui->searchLE->text(), Qt::CaseInsensitive))
                {
                    ProductWidget *p;
                    p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                    ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                    ++counter;
                }
            }
        }
        else
        {
            if (ui->departmentDB->currentText() == "Alimentos y Bebidas")
                pattern = 'A';
            else if (ui->departmentDB->currentText() == "Libros")
                pattern = 'L';
            else if (ui->departmentDB->currentText() == "Electrónicos")
                pattern = 'E';
            else if (ui->departmentDB->currentText() == "Hogar y Cocina")
                pattern = 'H';
            else
                pattern = 'D';

            for (int i(0); i < productsCopy.size(); ++i)
            {
                QJsonObject o = productsCopy[i].toObject();
                if (o["id"].toString()[0] == pattern && o["name"].toString().contains(ui->searchLE->text(), Qt::CaseInsensitive))
                {
                    ProductWidget *p;
                    p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
                    ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                    ++counter;
                }
            }
        }
    }

}
