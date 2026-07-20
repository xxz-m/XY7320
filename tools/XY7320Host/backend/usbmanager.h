#ifndef USBMANAGER_H
#define USBMANAGER_H
#include <QObject>
#include <QVariantList>
#include <QTimer>
class UsbManager:public QObject
{
    Q_OBJECT
    //表示把 C++ 里的磁盘列表暴露给 QML。
    Q_PROPERTY(QVariantList drives
                   READ drives
                       NOTIFY drivesChanged)

    Q_PROPERTY(QVariantList files
                   READ files
                       NOTIFY filesChanged)
    //给 QML 提供当前选中的 U 盘路径
    Q_PROPERTY(QString selectedDrive
                   READ selectedDrive
                       NOTIFY selectedDriveChanged)

    Q_PROPERTY(bool usbConnected
                   READ usbConnected
                       NOTIFY usbConnectedChanged)

public:
    explicit UsbManager(QObject *parent =nullptr);

    QVariantList drives() const;
    QVariantList files() const;
    QString selectedDrive() const;
    bool usbConnected() const;
    //表示 QML 可以调用这个函数。
    Q_INVOKABLE void refreshDrives();
    Q_INVOKABLE void selectDrive(const QString &path);
signals:
    //表示磁盘列表变化后通知 QML。
    void drivesChanged();
    void filesChanged();
    void selectedDriveChanged();
    void usbConnectedChanged();
private slots:
    void checkDrives();
private:
    void scanFiles();
    QString formatFileSize(qint64 bytes) const;
    QVariantList m_drives;
    QVariantList m_files;
    QStringList m_drivePaths;
    QString m_selectedDrive;
    bool m_usbConnected = false;
    QTimer m_refreshTimer;


};

#endif // USBMANAGER_H
