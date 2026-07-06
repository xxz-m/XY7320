/****************************************************************************
** Meta object code from reading C++ file 'serial_port_controller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/io/serial_port_controller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'serial_port_controller.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN20SerialPortControllerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN20SerialPortControllerE = QtMocHelpers::stringData(
    "SerialPortController",
    "portsChanged",
    "",
    "portNameChanged",
    "baudRateChanged",
    "dataBitsChanged",
    "parityChanged",
    "stopBitsChanged",
    "flowControlChanged",
    "isOpenChanged",
    "statusTextChanged",
    "errorTextChanged",
    "dataReceived",
    "data",
    "bytesWritten",
    "bytes",
    "errorOccurred",
    "message",
    "readAvailableData",
    "handleSerialError",
    "QSerialPort::SerialPortError",
    "error",
    "refreshPorts",
    "openPort",
    "closePort",
    "writeBytes",
    "ports",
    "QVariantList",
    "portName",
    "baudRate",
    "dataBits",
    "parity",
    "stopBits",
    "flowControl",
    "baudRateOptions",
    "dataBitsOptions",
    "parityOptions",
    "stopBitsOptions",
    "flowControlOptions",
    "isOpen",
    "statusText",
    "errorText"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN20SerialPortControllerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
      15,  157, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  128,    2, 0x06,   16 /* Public */,
       3,    0,  129,    2, 0x06,   17 /* Public */,
       4,    0,  130,    2, 0x06,   18 /* Public */,
       5,    0,  131,    2, 0x06,   19 /* Public */,
       6,    0,  132,    2, 0x06,   20 /* Public */,
       7,    0,  133,    2, 0x06,   21 /* Public */,
       8,    0,  134,    2, 0x06,   22 /* Public */,
       9,    0,  135,    2, 0x06,   23 /* Public */,
      10,    0,  136,    2, 0x06,   24 /* Public */,
      11,    0,  137,    2, 0x06,   25 /* Public */,
      12,    1,  138,    2, 0x06,   26 /* Public */,
      14,    1,  141,    2, 0x06,   28 /* Public */,
      16,    1,  144,    2, 0x06,   30 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      18,    0,  147,    2, 0x08,   32 /* Private */,
      19,    1,  148,    2, 0x08,   33 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      22,    0,  151,    2, 0x02,   35 /* Public */,
      23,    0,  152,    2, 0x02,   36 /* Public */,
      24,    0,  153,    2, 0x02,   37 /* Public */,
      25,    1,  154,    2, 0x02,   38 /* Public */,

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
    QMetaType::Void, QMetaType::QByteArray,   13,
    QMetaType::Void, QMetaType::LongLong,   15,
    QMetaType::Void, QMetaType::QString,   17,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 20,   21,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QByteArray,   13,

 // properties: name, type, flags, notifyId, revision
      26, 0x80000000 | 27, 0x00015009, uint(0), 0,
      28, QMetaType::QString, 0x00015103, uint(1), 0,
      29, QMetaType::Int, 0x00015103, uint(2), 0,
      30, QMetaType::Int, 0x00015103, uint(3), 0,
      31, QMetaType::Int, 0x00015103, uint(4), 0,
      32, QMetaType::Int, 0x00015103, uint(5), 0,
      33, QMetaType::Int, 0x00015103, uint(6), 0,
      34, 0x80000000 | 27, 0x00015409, uint(-1), 0,
      35, 0x80000000 | 27, 0x00015409, uint(-1), 0,
      36, 0x80000000 | 27, 0x00015409, uint(-1), 0,
      37, 0x80000000 | 27, 0x00015409, uint(-1), 0,
      38, 0x80000000 | 27, 0x00015409, uint(-1), 0,
      39, QMetaType::Bool, 0x00015001, uint(7), 0,
      40, QMetaType::QString, 0x00015001, uint(8), 0,
      41, QMetaType::QString, 0x00015001, uint(9), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SerialPortController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN20SerialPortControllerE.offsetsAndSizes,
    qt_meta_data_ZN20SerialPortControllerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN20SerialPortControllerE_t,
        // property 'ports'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'portName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'baudRate'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'dataBits'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'parity'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'stopBits'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'flowControl'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'baudRateOptions'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'dataBitsOptions'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'parityOptions'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'stopBitsOptions'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'flowControlOptions'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'isOpen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'statusText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'errorText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SerialPortController, std::true_type>,
        // method 'portsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'portNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'baudRateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dataBitsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'parityChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopBitsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'flowControlChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'isOpenChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statusTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'bytesWritten'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'readAvailableData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSerialError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSerialPort::SerialPortError, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openPort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'closePort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'writeBytes'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>
    >,
    nullptr
} };

void SerialPortController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialPortController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portsChanged(); break;
        case 1: _t->portNameChanged(); break;
        case 2: _t->baudRateChanged(); break;
        case 3: _t->dataBitsChanged(); break;
        case 4: _t->parityChanged(); break;
        case 5: _t->stopBitsChanged(); break;
        case 6: _t->flowControlChanged(); break;
        case 7: _t->isOpenChanged(); break;
        case 8: _t->statusTextChanged(); break;
        case 9: _t->errorTextChanged(); break;
        case 10: _t->dataReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 11: _t->bytesWritten((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 12: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->readAvailableData(); break;
        case 14: _t->handleSerialError((*reinterpret_cast< std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 15: _t->refreshPorts(); break;
        case 16: _t->openPort(); break;
        case 17: _t->closePort(); break;
        case 18: { bool _r = _t->writeBytes((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::portsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::portNameChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::baudRateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::dataBitsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::parityChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::stopBitsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::flowControlChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::isOpenChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::statusTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::errorTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)(const QByteArray & );
            if (_q_method_type _q_method = &SerialPortController::dataReceived; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)(qint64 );
            if (_q_method_type _q_method = &SerialPortController::bytesWritten; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)(const QString & );
            if (_q_method_type _q_method = &SerialPortController::errorOccurred; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
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
        case 3: *reinterpret_cast< int*>(_v) = _t->dataBits(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->parity(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->stopBits(); break;
        case 6: *reinterpret_cast< int*>(_v) = _t->flowControl(); break;
        case 7: *reinterpret_cast< QVariantList*>(_v) = _t->baudRateOptions(); break;
        case 8: *reinterpret_cast< QVariantList*>(_v) = _t->dataBitsOptions(); break;
        case 9: *reinterpret_cast< QVariantList*>(_v) = _t->parityOptions(); break;
        case 10: *reinterpret_cast< QVariantList*>(_v) = _t->stopBitsOptions(); break;
        case 11: *reinterpret_cast< QVariantList*>(_v) = _t->flowControlOptions(); break;
        case 12: *reinterpret_cast< bool*>(_v) = _t->isOpen(); break;
        case 13: *reinterpret_cast< QString*>(_v) = _t->statusText(); break;
        case 14: *reinterpret_cast< QString*>(_v) = _t->errorText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setPortName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setBaudRate(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->setDataBits(*reinterpret_cast< int*>(_v)); break;
        case 4: _t->setParity(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->setStopBits(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setFlowControl(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SerialPortController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialPortController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN20SerialPortControllerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialPortController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 19;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void SerialPortController::portsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SerialPortController::portNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SerialPortController::baudRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SerialPortController::dataBitsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SerialPortController::parityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SerialPortController::stopBitsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SerialPortController::flowControlChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SerialPortController::isOpenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SerialPortController::statusTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void SerialPortController::errorTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void SerialPortController::dataReceived(const QByteArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void SerialPortController::bytesWritten(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void SerialPortController::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}
QT_WARNING_POP
