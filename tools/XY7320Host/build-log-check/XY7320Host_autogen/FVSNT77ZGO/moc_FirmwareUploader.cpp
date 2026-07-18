/****************************************************************************
** Meta object code from reading C++ file 'FirmwareUploader.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/FirmwareUploader.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FirmwareUploader.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16FirmwareUploaderE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16FirmwareUploaderE = QtMocHelpers::stringData(
    "FirmwareUploader",
    "portsChanged",
    "",
    "portNameChanged",
    "baudRateChanged",
    "filePathChanged",
    "fileInfoChanged",
    "versionInfoChanged",
    "headerHexChanged",
    "packetSizeChanged",
    "headerDelayMsChanged",
    "packetDelayMsChanged",
    "progressChanged",
    "busyChanged",
    "serialOpenChanged",
    "manualHeaderSentChanged",
    "statusChanged",
    "logTextChanged",
    "sendNextPacket",
    "checkSelectedFile",
    "handleSerialReadyRead",
    "data",
    "appendRxLog",
    "handleHandshakeTimeout",
    "handleWriteFinished",
    "totalBytes",
    "tag",
    "handleSerialError",
    "error",
    "message",
    "handleWriteTimeout",
    "refreshPorts",
    "setFileUrl",
    "url",
    "openPort",
    "closePort",
    "start",
    "sendVersionFrameManual",
    "sendHeaderManual",
    "sendFirmwareManual",
    "copyHeaderToClipboard",
    "cancel",
    "clearLog",
    "ports",
    "QVariantList",
    "portName",
    "baudRate",
    "filePath",
    "fileName",
    "fileSizeText",
    "crcHex",
    "versionText",
    "versionFlag",
    "versionFrameHex",
    "headerHex",
    "packetSize",
    "headerDelayMs",
    "packetDelayMs",
    "progress",
    "busy",
    "serialOpen",
    "manualHeaderSent",
    "status",
    "logText"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16FirmwareUploaderE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      35,   14, // methods
      20,  275, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      16,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  224,    2, 0x06,   21 /* Public */,
       3,    0,  225,    2, 0x06,   22 /* Public */,
       4,    0,  226,    2, 0x06,   23 /* Public */,
       5,    0,  227,    2, 0x06,   24 /* Public */,
       6,    0,  228,    2, 0x06,   25 /* Public */,
       7,    0,  229,    2, 0x06,   26 /* Public */,
       8,    0,  230,    2, 0x06,   27 /* Public */,
       9,    0,  231,    2, 0x06,   28 /* Public */,
      10,    0,  232,    2, 0x06,   29 /* Public */,
      11,    0,  233,    2, 0x06,   30 /* Public */,
      12,    0,  234,    2, 0x06,   31 /* Public */,
      13,    0,  235,    2, 0x06,   32 /* Public */,
      14,    0,  236,    2, 0x06,   33 /* Public */,
      15,    0,  237,    2, 0x06,   34 /* Public */,
      16,    0,  238,    2, 0x06,   35 /* Public */,
      17,    0,  239,    2, 0x06,   36 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      18,    0,  240,    2, 0x08,   37 /* Private */,
      19,    0,  241,    2, 0x08,   38 /* Private */,
      20,    1,  242,    2, 0x08,   39 /* Private */,
      22,    1,  245,    2, 0x08,   41 /* Private */,
      23,    0,  248,    2, 0x08,   43 /* Private */,
      24,    2,  249,    2, 0x08,   44 /* Private */,
      27,    2,  254,    2, 0x08,   47 /* Private */,
      30,    1,  259,    2, 0x08,   50 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      31,    0,  262,    2, 0x02,   52 /* Public */,
      32,    1,  263,    2, 0x02,   53 /* Public */,
      34,    0,  266,    2, 0x02,   55 /* Public */,
      35,    0,  267,    2, 0x02,   56 /* Public */,
      36,    0,  268,    2, 0x02,   57 /* Public */,
      37,    0,  269,    2, 0x02,   58 /* Public */,
      38,    0,  270,    2, 0x02,   59 /* Public */,
      39,    0,  271,    2, 0x02,   60 /* Public */,
      40,    0,  272,    2, 0x02,   61 /* Public */,
      41,    0,  273,    2, 0x02,   62 /* Public */,
      42,    0,  274,    2, 0x02,   63 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   21,
    QMetaType::Void, QMetaType::QByteArray,   21,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong, QMetaType::Int,   25,   26,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   28,   29,
    QMetaType::Void, QMetaType::Int,   26,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QUrl,   33,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      43, 0x80000000 | 44, 0x00015009, uint(0), 0,
      45, QMetaType::QString, 0x00015103, uint(1), 0,
      46, QMetaType::Int, 0x00015103, uint(2), 0,
      47, QMetaType::QString, 0x00015103, uint(3), 0,
      48, QMetaType::QString, 0x00015001, uint(4), 0,
      49, QMetaType::QString, 0x00015001, uint(4), 0,
      50, QMetaType::QString, 0x00015001, uint(4), 0,
      51, QMetaType::QString, 0x00015103, uint(5), 0,
      52, QMetaType::Int, 0x00015103, uint(5), 0,
      53, QMetaType::QString, 0x00015001, uint(5), 0,
      54, QMetaType::QString, 0x00015001, uint(6), 0,
      55, QMetaType::Int, 0x00015103, uint(7), 0,
      56, QMetaType::Int, 0x00015103, uint(8), 0,
      57, QMetaType::Int, 0x00015103, uint(9), 0,
      58, QMetaType::Double, 0x00015001, uint(10), 0,
      59, QMetaType::Bool, 0x00015001, uint(11), 0,
      60, QMetaType::Bool, 0x00015001, uint(12), 0,
      61, QMetaType::Bool, 0x00015001, uint(13), 0,
      62, QMetaType::QString, 0x00015001, uint(14), 0,
      63, QMetaType::QString, 0x00015001, uint(15), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject FirmwareUploader::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN16FirmwareUploaderE.offsetsAndSizes,
    qt_meta_data_ZN16FirmwareUploaderE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16FirmwareUploaderE_t,
        // property 'ports'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'portName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'baudRate'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'filePath'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'fileName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'fileSizeText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'crcHex'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'versionText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'versionFlag'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'versionFrameHex'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'headerHex'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'packetSize'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'headerDelayMs'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'packetDelayMs'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'progress'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'busy'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'serialOpen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'manualHeaderSent'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'status'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'logText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FirmwareUploader, std::true_type>,
        // method 'portsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'portNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'baudRateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'filePathChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fileInfoChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'versionInfoChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'headerHexChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'packetSizeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'headerDelayMsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'packetDelayMsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'progressChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'busyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'serialOpenChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'manualHeaderSentChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'logTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendNextPacket'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkSelectedFile'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSerialReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'appendRxLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'handleHandshakeTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleWriteFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleSerialError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleWriteTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setFileUrl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUrl &, std::false_type>,
        // method 'openPort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'closePort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'start'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendVersionFrameManual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendHeaderManual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendFirmwareManual'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'copyHeaderToClipboard'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cancel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void FirmwareUploader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FirmwareUploader *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portsChanged(); break;
        case 1: _t->portNameChanged(); break;
        case 2: _t->baudRateChanged(); break;
        case 3: _t->filePathChanged(); break;
        case 4: _t->fileInfoChanged(); break;
        case 5: _t->versionInfoChanged(); break;
        case 6: _t->headerHexChanged(); break;
        case 7: _t->packetSizeChanged(); break;
        case 8: _t->headerDelayMsChanged(); break;
        case 9: _t->packetDelayMsChanged(); break;
        case 10: _t->progressChanged(); break;
        case 11: _t->busyChanged(); break;
        case 12: _t->serialOpenChanged(); break;
        case 13: _t->manualHeaderSentChanged(); break;
        case 14: _t->statusChanged(); break;
        case 15: _t->logTextChanged(); break;
        case 16: _t->sendNextPacket(); break;
        case 17: _t->checkSelectedFile(); break;
        case 18: _t->handleSerialReadyRead((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 19: _t->appendRxLog((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 20: _t->handleHandshakeTimeout(); break;
        case 21: _t->handleWriteFinished((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 22: _t->handleSerialError((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 23: _t->handleWriteTimeout((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->refreshPorts(); break;
        case 25: _t->setFileUrl((*reinterpret_cast< std::add_pointer_t<QUrl>>(_a[1]))); break;
        case 26: _t->openPort(); break;
        case 27: _t->closePort(); break;
        case 28: _t->start(); break;
        case 29: _t->sendVersionFrameManual(); break;
        case 30: _t->sendHeaderManual(); break;
        case 31: _t->sendFirmwareManual(); break;
        case 32: _t->copyHeaderToClipboard(); break;
        case 33: _t->cancel(); break;
        case 34: _t->clearLog(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::portsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::portNameChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::baudRateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::filePathChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::fileInfoChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::versionInfoChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::headerHexChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::packetSizeChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::headerDelayMsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::packetDelayMsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::progressChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::busyChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::serialOpenChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::manualHeaderSentChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::statusChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _q_method_type = void (FirmwareUploader::*)();
            if (_q_method_type _q_method = &FirmwareUploader::logTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QVariantList*>(_v) = _t->ports(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->portName(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->baudRate(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->filePath(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->fileName(); break;
        case 5: *reinterpret_cast< QString*>(_v) = _t->fileSizeText(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->crcHex(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->versionText(); break;
        case 8: *reinterpret_cast< int*>(_v) = _t->versionFlag(); break;
        case 9: *reinterpret_cast< QString*>(_v) = _t->versionFrameHex(); break;
        case 10: *reinterpret_cast< QString*>(_v) = _t->headerHex(); break;
        case 11: *reinterpret_cast< int*>(_v) = _t->packetSize(); break;
        case 12: *reinterpret_cast< int*>(_v) = _t->headerDelayMs(); break;
        case 13: *reinterpret_cast< int*>(_v) = _t->packetDelayMs(); break;
        case 14: *reinterpret_cast< double*>(_v) = _t->progress(); break;
        case 15: *reinterpret_cast< bool*>(_v) = _t->busy(); break;
        case 16: *reinterpret_cast< bool*>(_v) = _t->serialOpen(); break;
        case 17: *reinterpret_cast< bool*>(_v) = _t->manualHeaderSent(); break;
        case 18: *reinterpret_cast< QString*>(_v) = _t->status(); break;
        case 19: *reinterpret_cast< QString*>(_v) = _t->logText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setPortName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setBaudRate(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->setFilePath(*reinterpret_cast< QString*>(_v)); break;
        case 7: _t->setVersionText(*reinterpret_cast< QString*>(_v)); break;
        case 8: _t->setVersionFlag(*reinterpret_cast< int*>(_v)); break;
        case 11: _t->setPacketSize(*reinterpret_cast< int*>(_v)); break;
        case 12: _t->setHeaderDelayMs(*reinterpret_cast< int*>(_v)); break;
        case 13: _t->setPacketDelayMs(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *FirmwareUploader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FirmwareUploader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16FirmwareUploaderE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FirmwareUploader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 35)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 35;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 35)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 35;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void FirmwareUploader::portsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void FirmwareUploader::portNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void FirmwareUploader::baudRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void FirmwareUploader::filePathChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void FirmwareUploader::fileInfoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void FirmwareUploader::versionInfoChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void FirmwareUploader::headerHexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void FirmwareUploader::packetSizeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void FirmwareUploader::headerDelayMsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void FirmwareUploader::packetDelayMsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void FirmwareUploader::progressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void FirmwareUploader::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void FirmwareUploader::serialOpenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void FirmwareUploader::manualHeaderSentChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void FirmwareUploader::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void FirmwareUploader::logTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 15, nullptr);
}
QT_WARNING_POP
