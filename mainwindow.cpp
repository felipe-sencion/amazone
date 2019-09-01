#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
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
        u.setName(ui->newNameLE->text());
        u.setEmail(ui->newMailLE->text());
        u.setPassword(ui->newPasswordLE->text());
        users.push_back(u);
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
