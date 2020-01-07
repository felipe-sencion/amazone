#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QScrollArea>
#include <algorithm>
#include <QDateTime>

bool compare(const QJsonObject& lhs, QJsonObject& rhs)
{
    return lhs["sold"].toInt() < rhs["sold"].toInt();
}

std::priority_queue<QJsonObject, std::vector<QJsonObject>,
decltype(&compare)> pq(compare);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileMenu = ui->menuBar->addMenu(tr("&Archivo"));

    openFileAction = new QAction(tr("Abrir archivo"), this);
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
    fileMenu->addAction(openFileAction);
    qDebug()<<QDateTime::currentDateTime().toString("dd/MM/yy HH:mm:ss");
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
    //std::priority_queue<QJsonObject, std::vector<QJsonObject>,
    //                    decltype(&compare)> pq(compare);
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
        qDebug()<<"Las compras de " <<o["name"].toString() <<"Fueron: ";
        for (QJsonValueRef p : o["purchase"].toArray())
        {
            QStringList dates = p.toObject().keys();
            for (QString d : dates)
            {
                qDebug() <<d;
                QJsonArray boughtProducts = p.toObject()[d].toArray();
                for (int i(0); i < boughtProducts.size()-1; ++i)
                {
                    QString originName = boughtProducts[i].toObject()["id"].toString();
                    QHash<QString, int> edges;
                    if (grafo.contains(originName))
                        edges = grafo[originName];
                    qDebug() <<originName;
                    for (int j(i+1); j < boughtProducts.size(); ++j)
                    {
                        QHash<QString, int> symetricalEdge;
                        QString destinationName = boughtProducts[j].toObject()["id"].toString();
                        if (edges.contains(destinationName))
                            edges[destinationName]++;
                        else
                            edges[destinationName] = 1;
                        if (grafo.contains(destinationName))
                        {
                            symetricalEdge = grafo[destinationName];
                            if (symetricalEdge.contains(originName))
                                symetricalEdge[originName]++;
                            else
                                symetricalEdge[originName] = 1;
                        }
                        else
                            symetricalEdge[originName] = 1;
                        qDebug()<<"\tcon " <<destinationName;
                        grafo[destinationName] = symetricalEdge;

                    }
                    grafo[originName] = edges;
                }
            }
        }
        users.push_back(u);
    }
    QHash<QString, QHash<QString, int> >::iterator originIterator;
    originIterator = grafo.begin();
    while(originIterator != grafo.end())
    {
        QHash<QString, int>::iterator destinationIterator;
        destinationIterator = originIterator.value().begin();
        qDebug()<<originIterator.key();
        while(destinationIterator != originIterator.value().end())
        {
            qDebug()<<"\t" <<destinationIterator.key()
                   <<":" <<destinationIterator.value();
            ++destinationIterator;
        }
        ++originIterator;
    }


    for (int i(0); i < products.size(); ++i)
    {
        QJsonObject o = products[i].toObject();
        ProductWidget *p;
        p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->auxScrollArea);
        connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
        p->setId(o["id"].toString());
        p->setDescription(o["name"].toString());
        p->setSold(o["sold"].toInt());
        ui->auxGrid->addWidget(p, i/3, i%3, Qt::AlignCenter);
        pq.push(o);
    }
    for (int i(0); i < 3; ++i)
    {
        ProductWidget *p;
        QJsonObject o = pq.top();
        pq.pop();
        p = new ProductWidget(o["id"].toString(), o["name"].toString() + "\n\n$" + QString::number(o["price"].toDouble()), ui->recommendationSA);
        connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
        p->setId(o["id"].toString());
        p->setDescription(o["name"].toString());
        p->setSold(o["sold"].toInt());
        ui->horizontalLayout->addWidget(p, 0);
    }
    dbFile.close();
}

void MainWindow::saveDB()
{
    dbFile.open(QIODevice::WriteOnly);
    QJsonObject jsonObj;
    QJsonDocument jsonDoc;
    QJsonObject o = jsonDB[userIndex].toObject();
    if (newPurchase.size() > 0)
    {
        QJsonObject pO;
        pO[QDateTime::currentDateTime().toString("dd/MM/yy HH:mm:ss")] = newPurchase;
        purchase.append(pO);
    }
    o["purchase"] = purchase;
    jsonDB[userIndex] = o;
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

QJsonObject MainWindow::getProductById(QString id)
{
    QJsonObject obj;
    qDebug()<<"Buscando: " <<id;
    for(int i(0); i < productsCopy.size(); ++i)
    {
        if(productsCopy[i].toObject()["id"].toString() == id)
        {
            obj = productsCopy[i].toObject();
            qDebug()<<"Encontrado: ";
            break;
        }
    }
    return obj;
}

void MainWindow::clearRecommendations()
{
    QLayoutItem* item;
    while( (item = ui->horizontalLayout->takeAt(0)) )
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
                this->setMinimumSize(1200, 700);
                userIndex = i;
                purchase = jsonDB[i].toObject()["purchase"].toArray();
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
            connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
            p->setId(o["id"].toString());
            p->setDescription(o["name"].toString());
            p->setSold(o["sold"].toInt());
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
                connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                p->setId(o["id"].toString());
                p->setDescription(o["name"].toString());
                p->setSold(o["sold"].toInt());
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
                connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                p->setId(o["id"].toString());
                p->setDescription(o["name"].toString());
                p->setSold(o["sold"].toInt());
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
                connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                p->setId(o["id"].toString());
                p->setDescription(o["name"].toString());
                p->setSold(o["sold"].toInt());
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
                connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                p->setId(o["id"].toString());
                p->setDescription(o["name"].toString());
                p->setSold(o["sold"].toInt());
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
                    connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                    p->setId(o["id"].toString());
                    p->setDescription(o["name"].toString());
                    p->setSold(o["sold"].toInt());
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
                    connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                    p->setId(o["id"].toString());
                    p->setDescription(o["name"].toString());
                    p->setSold(o["sold"].toInt());
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
                    connect(p, SIGNAL(added(int)), this, SLOT(addProduct(int)));
                    p->setId(o["id"].toString());
                    p->setDescription(o["name"].toString());
                    p->setSold(o["sold"].toInt());
                    ui->auxGrid->addWidget(p, counter/3, counter%3, Qt::AlignCenter);
                    ++counter;
                }
            }
        }
    }

}

void MainWindow::addProduct(int amount)
{
    ui->descriptionLabel->setText("Tal vez te interese:");
    int displayedRecommendations = 0;
    //qDebug()<<"add: " << static_cast<ProductWidget*>(sender())->getDescription();
    ProductWidget* p = static_cast<ProductWidget*>(sender());
    QJsonObject op = getProductById(p->getId());
    QString senderName = p->getId();
    QJsonObject o;
    o["id"] = op["id"].toString();
    //purchase.append(o);
    newPurchase.append(o);
    for (int i(0); i < products.size(); ++i)
    {
        o = products[i].toObject();
        if (o["id"] == p->getId())
        {
            o["sold"] = op["sold"].toInt() + amount;
            products[i] = o;
            break;
        }
    }

    clearRecommendations();
    while(!pq.empty())
        pq.pop();
    qDebug()<<pq.size();

    o = getProductById(p->getId());
    //qDebug()<<"El tamaño de purchase es: " <<newPurchase.size();
    //qDebug()<<"En cola agregando:";
    for (int i(0); i < newPurchase.size(); ++i)
    {
        QJsonObject cartItem = newPurchase[i].toObject();
        qDebug()<<"cartItemId: " <<cartItem["id"].toString();
        QHash<QString, int> recommendationsHash = grafo[cartItem["id"].toString()];
        QHash<QString, int>::Iterator it = recommendationsHash.begin();
        while (it != recommendationsHash.end())
        {
            //qDebug()<<"ESTE ES UN MENSAJE MUY IMPORTANTE: " <<it.value();
            QJsonObject rec = getProductById(it.key());
            rec["sold"] = it.value();
            //pq.push(getProductById(it.key()));
            pq.push(rec);
            qDebug()<<cartItem["id"].toString() <<"," <<it.key() <<":" <<it.value();
            ++it;
        }
    }
    QStringList added;
    while(!pq.empty() && displayedRecommendations < 13)
    {
        QJsonObject item = pq.top();
        //qDebug()<<"En top " <<item["id"];
        pq.pop();
        QJsonObject temp;
        temp["id"] = item["id"].toString();
        if (item["id"].toString() != senderName &&
                !added.contains(item["id"].toString()) &&
                !newPurchase.contains(temp))
        {
            //qDebug()<<"Added size: " <<added.size();
            ProductWidget* w;
            w = new ProductWidget(item["id"].toString(), item["name"].toString() + "\n\n$" + QString::number(item["price"].toDouble()), ui->recommendationSA);
            connect(w, SIGNAL(added(int)), this, SLOT(addProduct(int)));
            w->setId(item["id"].toString());
            w->setDescription(item["name"].toString());
            w->setSold(item["sold"].toInt());
            ui->horizontalLayout->addWidget(w, 0);
            qDebug()<<"En recomendaciones:" <<w->getId() <<"," <<w->getDescription() <<"," <<w->getSold();
            ++displayedRecommendations;
            added.push_back(item["id"].toString());
            //qDebug()<<"Added: " <<item["id"].toString();
        }
    }


}
