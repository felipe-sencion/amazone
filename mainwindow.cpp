#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

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

    dbFile.close();
}

void MainWindow::saveDB()
{
    dbFile.open(QIODevice::WriteOnly);
    QJsonObject jsonObj;
    QJsonDocument jsonDoc;
    jsonObj["users"] = jsonDB;
    jsonDoc = QJsonDocument(jsonObj);

    dbFile.write(jsonDoc.toJson());
    dbFile.close();
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
