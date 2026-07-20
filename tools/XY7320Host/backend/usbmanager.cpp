#include "usbmanager.h"
#include "QDebug"
#include <QDir>
#include <QStorageInfo>
#include <QVariantMap>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

UsbManager::UsbManager(QObject *parent)
    : QObject(parent)
{
    m_refreshTimer.setInterval(1000);
    connect(&m_refreshTimer,
            &QTimer::timeout,
            this,
            &UsbManager::checkDrives);
    m_refreshTimer.start();
    checkDrives();
}

void UsbManager::refreshDrives()
{
    QVariantList drives;
    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for(const QStorageInfo &storage : volumes){
        if (!storage.isValid() || !storage.isReady()) {
            continue;
        }
        const QString rootPath = storage.rootPath();
        if (rootPath.isEmpty()) {
            continue;
        }
#ifdef Q_OS_WIN
        if (GetDriveTypeW(reinterpret_cast<LPCWSTR>(rootPath.utf16())) != DRIVE_REMOVABLE) {
            continue;
        }
#endif
        QVariantMap drive;
        drive.insert(QStringLiteral("name"), storage.displayName());
        drive.insert(QStringLiteral("path"), rootPath);
        drive.insert(QStringLiteral("totalBytes"), storage.bytesTotal());
        drive.insert(QStringLiteral("availableBytes"), storage.bytesAvailable());
        drives.append(drive);
    }
    m_drives = drives;
    emit drivesChanged();
    scanFiles();
}

void UsbManager::checkDrives()
{
    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();

    QStringList currentPaths;

    for (const QStorageInfo &storage : volumes) {
        if (!storage.isValid() || !storage.isReady()) {
            continue;
        }

        currentPaths.append(storage.rootPath());
    }

    currentPaths.sort();

    bool usbConnected = false;
#ifdef Q_OS_WIN
    for (const QString &path : currentPaths) {
        const UINT driveType = GetDriveTypeW(
            reinterpret_cast<LPCWSTR>(path.utf16())
            );
        if (driveType == DRIVE_REMOVABLE) {
            usbConnected = true;
            break;
        }
    }
#endif

    if (m_usbConnected != usbConnected) {
        m_usbConnected = usbConnected;
        emit usbConnectedChanged();
    }

    if (currentPaths == m_drivePaths) {
        return;
    }

    m_drivePaths = currentPaths;
    refreshDrives();
}
QString UsbManager::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }

    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB")
        .arg(static_cast<double>(bytes) / 1024.0, 0, 'f', 1);
    }

    return QStringLiteral("%1 MB")
        .arg(static_cast<double>(bytes) / (1024.0 * 1024.0), 0, 'f', 1);
}

void UsbManager::scanFiles()
{
    QVariantList files;
    QStringList paths;

    if (!m_selectedDrive.isEmpty()) {
        paths.append(m_selectedDrive);
    } else {
        for (const QVariant &driveValue : m_drives) {
            const QVariantMap drive = driveValue.toMap();
            const QString path = drive.value(QStringLiteral("path")).toString();
            if (!path.isEmpty()) {
                paths.append(path);
            }
        }
    }

    for (const QString &path : paths) {
        const QDir directory(path);
        const QFileInfoList fileInfoList = directory.entryInfoList(
            QStringList() << "*.bin",
            QDir::Files | QDir::Readable,
            QDir::Name
            );

        for (const QFileInfo &fileInfo : fileInfoList) {
            QVariantMap file;
            file.insert(QStringLiteral("name"), fileInfo.fileName());
            file.insert(QStringLiteral("path"), fileInfo.absoluteFilePath());
            file.insert(QStringLiteral("size"), fileInfo.size());
            file.insert(
                QStringLiteral("sizeText"),
                formatFileSize(fileInfo.size())
                );
            file.insert(
                QStringLiteral("modifiedText"),
                fileInfo.lastModified().toString(
                    QStringLiteral("yyyy-MM-dd HH:mm:ss")
                    )
                );
            files.append(file);
        }
    }

    m_files = files;
    emit filesChanged();
}
void UsbManager::selectDrive(const QString &path)
{
    const QString cleanPath = QDir::fromNativeSeparators(path);

    if (cleanPath.isEmpty()) {
        return;
    }

    QStorageInfo storage(cleanPath);

    if (!storage.isValid() || !storage.isReady()) {
        m_files.clear();
        emit filesChanged();
        return;
    }

    if (m_selectedDrive != cleanPath) {
        m_selectedDrive = cleanPath;
        emit selectedDriveChanged();
    }

    scanFiles();
}

QVariantList UsbManager::drives() const
{
    return m_drives;
}

QVariantList UsbManager::files() const
{
    return m_files;
}

QString UsbManager::selectedDrive() const
{
    return m_selectedDrive;
}

bool UsbManager::usbConnected() const
{
    return m_usbConnected;
}
