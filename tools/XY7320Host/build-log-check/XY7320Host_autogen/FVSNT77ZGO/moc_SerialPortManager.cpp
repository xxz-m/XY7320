/****************************************************************************
** Meta object code from reading C++ file 'SerialPortManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/SerialPortManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SerialPortManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17SerialPortManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN17SerialPortManagerE = QtMocHelpers::stringData(
    "SerialPortManager",
    "portsChanged",
    "",
    "portNameChanged",
    "baudRateChanged",
    "isOpenChanged",
    "dataReceived",
    "data",
    "writeFinished",
    "totalBytes",
    "SerialPortManager::WriteTag",
    "tag",
    "writeTimeout",
    "serialErrorOccurred",
    "QSerialPort::SerialPortError",
    "error",
    "message",
    "handleReadyRead",
    "handleBytesWritten",
    "bytes",
    "handleError",
    "handleWriteTimeout",
    "refreshPorts",
    "open",
    "close",
    "clearRxBuffer",
    "ports",
    "QVariantList",
    "portName",
    "baudRate",
    "isOpen",
    "WriteTag",
    "DebugTx",
    "UpgradeHandshake",
    "UpgradeHeader",
    "UpgradePacket",
    "UpgradeFinish",
    "Unknown"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN17SerialPortManagerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       4,  142, // properties
       1,  162, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  110,    2, 0x06,    6 /* Public */,
       3,    0,  111,    2, 0x06,    7 /* Public */,
       4,    0,  112,    2, 0x06,    8 /* Public */,
       5,    0,  113,    2, 0x06,    9 /* Public */,
       6,    1,  114,    2, 0x06,   10 /* Public */,
       8,    2,  117,    2, 0x06,   12 /* Public */,
      12,    1,  122,    2, 0x06,   15 /* Public */,
      13,    2,  125,    2, 0x06,   17 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      17,    0,  130,    2, 0x08,   20 /* Private */,
      18,    1,  131,    2, 0x08,   21 /* Private */,
      20,    1,  134,    2, 0x08,   23 /* Private */,
      21,    0,  137,    2, 0x08,   25 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      22,    0,  138,    2, 0x02,   26 /* Public */,
      23,    0,  139,    2, 0x02,   27 /* Public */,
      24,    0,  140,    2, 0x02,   28 /* Public */,
      25,    0,  141,    2, 0x02,   29 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    7,
    QMetaType::Void, QMetaType::LongLong, 0x80000000 | 10,    9,   11,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 14, QMetaType::QString,   15,   16,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   19,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      26, 0x80000000 | 27, 0x00015009, uint(0), 0,
      28, QMetaType::QString, 0x00015103, uint(1), 0,
      29, QMetaType::Int, 0x00015103, uint(2), 0,
      30, QMetaType::Bool, 0x00015001, uint(3), 0,

 // enums: name, alias, flags, count, data
      31,   31, 0x2,    6,  167,

 // enum data: key, value
      32, uint(SerialPortManager::WriteTag::DebugTx),
      33, uint(SerialPortManager::WriteTag::UpgradeHandshake),
      34, uint(SerialPortManager::WriteTag::UpgradeHeader),
      35, uint(SerialPortManager::WriteTag::UpgradePacket),
      36, uint(SerialPortManager::WriteTag::UpgradeFinish),
      37, uint(SerialPortManager::WriteTag::Unknown),

       0        // eod
};

Q_CONSTINIT const QMetaObject SerialPortManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN17SerialPortManagerE.offsetsAndSizes,
    qt_meta_data_ZN17SerialPortManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN17SerialPortManagerE_t,
        // property 'ports'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'portName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'baudRate'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'isOpen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // enum 'WriteTag'
        QtPrivate::TypeAndForceComplete<SerialPortManager::WriteTag, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SerialPortManager, std::true_type>,
        // method 'portsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'portNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'baudRateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'isOpenChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'writeFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<SerialPortManager::WriteTag, std::false_type>,
        // method 'writeTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<SerialPortManager::WriteTag, std::false_type>,
        // method 'serialErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSerialPort::SerialPortError, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'handleReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleBytesWritten'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'handleError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSerialPort::SerialPortError, std::false_type>,
        // method 'handleWriteTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'open'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'close'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearRxBuffer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void SerialPortManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialPortManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portsChanged(); break;
        case 1: _t->portNameChanged(); break;
        case 2: _t->baudRateChanged(); break;
        case 3: _t->isOpenChanged(); break;
        case 4: _t->dataReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 5: _t->writeFinished((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<SerialPortManager::WriteTag>>(_a[2]))); break;
        case 6: _t->writeTimeout((*reinterpret_cast< std::add_pointer_t<SerialPortManager::WriteTag>>(_a[1]))); break;
        case 7: _t->serialErrorOccurred((*reinterpret_cast< std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 8: _t->handleReadyRead(); break;
        case 9: _t->handleBytesWritten((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 10: _t->handleError((*reinterpret_cast< std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 11: _t->handleWriteTimeout(); break;
        case 12: _t->refreshPorts(); break;
        case 13: { bool _r = _t->open();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 14: _t->close(); break;
        case 15: _t->clearRxBuffer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (SerialPortManager::*)();
            if (_q_method_type _q_method = &SerialPortManager::portsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)();
            if (_q_method_type _q_method = &SerialPortManager::portNameChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)();
            if (_q_method_type _q_method = &SerialPortManager::baudRateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)();
            if (_q_method_type _q_method = &SerialPortManager::isOpenChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)(const QByteArray & );
            if (_q_method_type _q_method = &SerialPortManager::dataReceived; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)(qint64 , SerialPortManager::WriteTag );
            if (_q_method_type _q_method = &SerialPortManager::writeFinished; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)(SerialPortManager::WriteTag );
            if (_q_method_type _q_method = &SerialPortManager::writeTimeout; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortManager::*)(QSerialPort::SerialPortError , const QString & );
            if (_q_method_type _q_method = &SerialPortManager::serialErrorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
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
        case 3: *reinterpret_cast< bool*>(_v) = _t->isOpen(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setPortName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setBaudRate(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SerialPortManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialPortManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN17SerialPortManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialPortManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 16;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void SerialPortManager::portsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SerialPortManager::portNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SerialPortManager::baudRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SerialPortManager::isOpenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SerialPortManager::dataReceived(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void SerialPortManager::writeFinished(qint64 _t1, SerialPortManager::WriteTag _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void SerialPortManager::writeTimeout(SerialPortManager::WriteTag _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void SerialPortManager::serialErrorOccurred(QSerialPort::SerialPortError _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
