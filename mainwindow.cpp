#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <stdio.h>

#include <string>
#include <iostream>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QVariantMap>
#include <QRegularExpression>
#include <QMessageBox>
#include <QByteArray>
//#include <QtNetwork/QUdpSocket>
//#include <QtNetwork/QHostAddress>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>




extern QJsonObject dataJSON;


QJsonObject dataJSON;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(290, 320);

    getDataAsJSON();



    //QPushButton* addMacButtonSave = this->ui->addMacButtonSave;
    connect(this->ui->addMacButtonSave, &QPushButton::clicked, this, &MainWindow::addMacAddress);

    connect(this->ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::tabSwitched);

    this->ui->addMacLabelInfo->setText("");
    this->ui->addMacLabelInfo->setStyleSheet("color: red;");

    connect(this->ui->mainListMacs, &QComboBox::currentIndexChanged, this, &MainWindow::changeListObj);

    this->ui->mainLabelMacAddress->setText("");

    connect(this->ui->mainButtonSendPacket, &QPushButton::clicked, this, &MainWindow::sendMagicPacket);

    connect(this->ui->mainButtonDelete, &QPushButton::clicked, this, &MainWindow::deleteEntry);
    this->ui->mainButtonDelete->setDisabled(true);
    this->ui->mainButtonSendPacket->setDisabled(true);


    std::cout << "ON!" << std::endl;

    MainWindow::loadMainPageData();





}





MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::getDataAsJSON()
{
    QString home = QDir::homePath();
    QString filepath = home + "/MagicPacketSender/";
    QString file = filepath + "data.json";

    if (!QDir(filepath).exists())
    {
        QDir().mkdir(filepath);
    }

    if (!QFile(file).exists())
    {
        QFile f(file);
        if (f.open(QIODevice::WriteOnly))
        {
            f.write("{}");
            f.close();
        }
    }

    QString data = "{}";
    QFile f(file);
    if (f.open(QIODevice::ReadOnly))
    {
        data = f.readAll();
        f.close();
    }

    QJsonObject json = QJsonDocument::fromJson(data.toUtf8()).object();

    if (!json.contains("savedAddresses"))
    {
        json.insert("savedAddresses", QJsonObject());
    }

    dataJSON = json;
    return;
}


void MainWindow::saveDataFromJSON(QJsonObject data)
{
    QJsonDocument jsonDoc(data);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    // Speichern der Daten in einer Datei
    QString home = QDir::homePath();
    QString filepath = home + "/MagicPacketSender/";
    QString filename = filepath + "data.json";

    if (!QDir(filepath).exists()) {
        QDir().mkpath(filepath);
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Fehler beim Öffnen der Datei " << filename;
        return;
    }

    file.write(jsonData);
    file.close();

    dataJSON = data;
    loadMainPageData();
}



void MainWindow::loadMainPageData() {
    MainWindow::getDataAsJSON();
    QJsonObject data = dataJSON;

    ui->mainListMacs->clear();

    QJsonObject savedAddresses = data["savedAddresses"].toObject();
    foreach(const QString& key, savedAddresses.keys()) {
        ui->mainListMacs->addItem(key);
    }

    if (ui->mainListMacs->count() == 0) {
        ui->mainButtonDelete->setDisabled(true);
        ui->mainButtonSendPacket->setDisabled(true);
    } else {
        ui->mainButtonDelete->setDisabled(false);
        ui->mainButtonSendPacket->setDisabled(false);
    }
}


void MainWindow::tabSwitched()
{
    if (ui->tabWidget->currentWidget()->objectName() == "mainTab") {
        loadMainPageData();
    }
}


QString MainWindow::getMacFromName(const QString& name)
{
    QString mac;
    QJsonObject data = dataJSON;
    if (data["savedAddresses"].isObject() && data["savedAddresses"].toObject().contains(name)) {
        mac = data["savedAddresses"].toObject()[name].toString();
    }
    return mac;
}


bool MainWindow::isMacAddress(const QString& macAddress)
{
    QRegularExpression pattern("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$");
    return pattern.match(macAddress).hasMatch();
}


void MainWindow::sendMagicPacket()
{
    if (ui->mainListMacs->count() != 0) {
        QString curr = ui->mainListMacs->currentText();
        if (dataJSON.contains("savedAddresses")) {
            QJsonObject savedAddresses = dataJSON["savedAddresses"].toObject();
            if (savedAddresses.contains(curr)) {
                QString mac = savedAddresses[curr].toString();
                QByteArray macBytes;
                macBytes = QByteArray::fromHex(mac.toUtf8());


                wake_on_lan(macBytes.data());
                //send_magic_packet(macBytes.data(), macBytes.size());
                QMessageBox::information(this, tr("Erfolg"), tr("Es wurde erfolgreich ein Magic Packet an %1 (%2) gesendet.").arg(curr, mac));
            }
        }
    }
}







void MainWindow::addMacAddress()
{
    QString macAddress = ui->addMacLineMacAddress->text();
    QString name = ui->addMacLineName->text();

    ui->addMacLabelInfo->setText("");
    ui->addMacLabelInfo->setStyleSheet("color: white;");

    if (!name.isEmpty() && !macAddress.isEmpty() && !name.trimmed().isEmpty() && !macAddress.trimmed().isEmpty()) {
        if (isMacAddress(macAddress)) {
            QJsonObject data = dataJSON;

            if (!data.contains("savedAddresses")) {
                data["savedAddresses"] = QJsonObject();
            }

            if (!data["savedAddresses"].toObject().contains(name)) {
                QJsonObject savedAddresses = data["savedAddresses"].toObject();
                savedAddresses[name] = macAddress;
                data["savedAddresses"] = savedAddresses;




                saveDataFromJSON(data);

                ui->tabWidget->setCurrentIndex(0);
                ui->addMacLineMacAddress->setText("");
                ui->addMacLineName->setText("");
            } else {
                ui->addMacLabelInfo->setText("Ein Eintrag mit diesem Name existiert bereits!");
                ui->addMacLabelInfo->setStyleSheet("color: red;");
            }
        } else {
            ui->addMacLabelInfo->setText("Die eingetragene MAC-Adresse ist ungültig!");
            ui->addMacLabelInfo->setStyleSheet("color: red;");
        }
    } else {
        ui->addMacLabelInfo->setText("Es müssen alle Felder ausgefüllt sein!");
        ui->addMacLabelInfo->setStyleSheet("color: red;");
    }
}




void MainWindow::changeListObj() {
QString curr = ui->mainListMacs->currentText();
ui->mainLabelMacAddress->setText(getMacFromName(curr));

if (ui->mainListMacs->count() == 0) {
    ui->mainButtonDelete->setDisabled(true);
    ui->mainButtonSendPacket->setDisabled(true);
} else {
    ui->mainButtonDelete->setDisabled(false);
    ui->mainButtonSendPacket->setDisabled(false);
}

}




void MainWindow::deleteEntry() {
    QMessageBox qm;
    qm.setIcon(QMessageBox::Warning);
    qm.setWindowTitle("Eintrag löschen");
    qm.setText("Möchtest du diesen Eintrag wirklich löschen?");
    qm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int ret = qm.exec();

    if (ret == QMessageBox::Yes) {
        if (ui->mainListMacs->count() != 0) {
            QJsonObject data = dataJSON;

            QString curr = ui->mainListMacs->currentText();
            QJsonObject savedAdresses = data["savedAddresses"].toObject();
            savedAdresses.remove(QString(curr));
            data["savedAddresses"] = savedAdresses;

            saveDataFromJSON(data);
            loadMainPageData();
        }
    }

}



void MainWindow::wake_on_lan(const char *mac_addr)
{
    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_fd < 0) {
        std::cerr << "Error: socket creation failed" << std::endl;
        return;
    }

    int broadcast = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        std::cerr << "Error: failed to set socket option" << std::endl;
        ::close(sock_fd);
        return;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9);
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    char packet[102];
    memset(packet, 0xff, 6);
    for (int i = 1; i <= 16; i++) {
        for (int j = 0; j < 6; j++) {
            packet[i * 6 + j] = mac_addr[j];
        }
    }

    if (sendto(sock_fd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Error: failed to send wake-on-lan packet" << std::endl;
    }

    ::close(sock_fd);
}
