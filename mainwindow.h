#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "user.h"
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_emailLE_textChanged(const QString &arg1);

    void on_passwordLE_textChanged(const QString &arg1);

    void on_loginPB_clicked();

    void on_newUserPB_clicked();

    void on_newNameLE_textChanged(const QString &arg1);

    void on_newMailLE_textChanged(const QString &arg1);

    void on_newPasswordLE_textChanged(const QString &arg1);

    void on_createPB_clicked();

    void openFile();

private:
    Ui::MainWindow *ui;
    QString fileName;
    QMenu *fileMenu;
    QAction *openFileAction;
    vector<User> users;
    QFile dbFile;
    QJsonArray jsonDB;

    void enableLoginPB();
    void enableCreatePB();
    void loadDB();
    void saveDB();
};

#endif // MAINWINDOW_H
