#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void getDataAsJSON();

    void saveDataFromJSON(QJsonObject data);

    void loadMainPageData();

    void tabSwitched();

    QString getMacFromName(const QString& name);

    bool isMacAddress(const QString& macAddress);

    void sendMagicPacket();

    void wake_on_lan(const char *mac_addr);

    void addMacAddress();

    void changeListObj();

    void deleteEntry();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
