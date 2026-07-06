/****************************************************************************
** Meta object code from reading C++ file 'serial_port_controller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/serial_port_controller.h"
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
    "receivedTextChanged",
    "lineEndingChanged",
    "autoScrollChanged",
    "sendSucceeded",
    "bytes",
    "sendFailed",
    "message",
    "refreshPorts",
    "openPort",
    "closePort",
    "sendText",
    "text",
    "clearReceived",
    "handleReadyRead",
    "flushReceivedData",
    "handleSerialError",
    "QSerialPort::SerialPortError",
    "error",
    "handleBytesWritten",
    "ports",
    "QVariantList",
    "baudRateOptions",
    "dataBitsOptions",
    "parityOptions",
    "stopBitsOptions",
    "flowControlOptions",
    "lineEndingOptions",
    "portName",
    "baudRate",
    "dataBits",
    "parity",
    "stopBits",
    "flowControl",
    "isOpen",
    "statusText",
    "errorText",
    "receivedText",
    "lineEnding",
    "autoScroll"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN20SerialPortControllerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
      19,  192, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      15,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  158,    2, 0x06,   20 /* Public */,
       3,    0,  159,    2, 0x06,   21 /* Public */,
       4,    0,  160,    2, 0x06,   22 /* Public */,
       5,    0,  161,    2, 0x06,   23 /* Public */,
       6,    0,  162,    2, 0x06,   24 /* Public */,
       7,    0,  163,    2, 0x06,   25 /* Public */,
       8,    0,  164,    2, 0x06,   26 /* Public */,
       9,    0,  165,    2, 0x06,   27 /* Public */,
      10,    0,  166,    2, 0x06,   28 /* Public */,
      11,    0,  167,    2, 0x06,   29 /* Public */,
      12,    0,  168,    2, 0x06,   30 /* Public */,
      13,    0,  169,    2, 0x06,   31 /* Public */,
      14,    0,  170,    2, 0x06,   32 /* Public */,
      15,    1,  171,    2, 0x06,   33 /* Public */,
      17,    1,  174,    2, 0x06,   35 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      19,    0,  177,    2, 0x0a,   37 /* Public */,
      20,    0,  178,    2, 0x0a,   38 /* Public */,
      21,    0,  179,    2, 0x0a,   39 /* Public */,
      22,    1,  180,    2, 0x0a,   40 /* Public */,
      24,    0,  183,    2, 0x0a,   42 /* Public */,
      25,    0,  184,    2, 0x08,   43 /* Private */,
      26,    0,  185,    2, 0x08,   44 /* Private */,
      27,    1,  186,    2, 0x08,   45 /* Private */,
      30,    1,  189,    2, 0x08,   47 /* Private */,

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
    QMetaType::Void, QMetaType::LongLong,   16,
    QMetaType::Void, QMetaType::QString,   18,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 28,   29,
    QMetaType::Void, QMetaType::LongLong,   16,

 // properties: name, type, flags, notifyId, revision
      31, 0x80000000 | 32, 0x00015009, uint(0), 0,
      33, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      34, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      35, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      36, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      37, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      38, 0x80000000 | 32, 0x00015409, uint(-1), 0,
      39, QMetaType::QString, 0x00015103, uint(1), 0,
      40, QMetaType::Int, 0x00015103, uint(2), 0,
      41, QMetaType::Int, 0x00015103, uint(3), 0,
      42, QMetaType::Int, 0x00015103, uint(4), 0,
      43, QMetaType::Int, 0x00015103, uint(5), 0,
      44, QMetaType::Int, 0x00015103, uint(6), 0,
      45, QMetaType::Bool, 0x00015001, uint(7), 0,
      46, QMetaType::QString, 0x00015001, uint(8), 0,
      47, QMetaType::QString, 0x00015001, uint(9), 0,
      48, QMetaType::QString, 0x00015001, uint(10), 0,
      49, QMetaType::Int, 0x00015103, uint(11), 0,
      50, QMetaType::Bool, 0x00015103, uint(12), 0,

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
        // property 'lineEndingOptions'
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
        // property 'isOpen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'statusText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'errorText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'receivedText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'lineEnding'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'autoScroll'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
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
        // method 'receivedTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lineEndingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'autoScrollChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendSucceeded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'sendFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'openPort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'closePort'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendText'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'clearReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'flushReceivedData'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSerialError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSerialPort::SerialPortError, std::false_type>,
        // method 'handleBytesWritten'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>
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
        case 10: _t->receivedTextChanged(); break;
        case 11: _t->lineEndingChanged(); break;
        case 12: _t->autoScrollChanged(); break;
        case 13: _t->sendSucceeded((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 14: _t->sendFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->refreshPorts(); break;
        case 16: _t->openPort(); break;
        case 17: _t->closePort(); break;
        case 18: _t->sendText((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 19: _t->clearReceived(); break;
        case 20: _t->handleReadyRead(); break;
        case 21: _t->flushReceivedData(); break;
        case 22: _t->handleSerialError((*reinterpret_cast< std::add_pointer_t<QSerialPort::SerialPortError>>(_a[1]))); break;
        case 23: _t->handleBytesWritten((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
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
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::receivedTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::lineEndingChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)();
            if (_q_method_type _q_method = &SerialPortController::autoScrollChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)(qint64 );
            if (_q_method_type _q_method = &SerialPortController::sendSucceeded; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _q_method_type = void (SerialPortController::*)(const QString & );
            if (_q_method_type _q_method = &SerialPortController::sendFailed; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QVariantList*>(_v) = _t->ports(); break;
        case 1: *reinterpret_cast< QVariantList*>(_v) = _t->baudRateOptions(); break;
        case 2: *reinterpret_cast< QVariantList*>(_v) = _t->dataBitsOptions(); break;
        case 3: *reinterpret_cast< QVariantList*>(_v) = _t->parityOptions(); break;
        case 4: *reinterpret_cast< QVariantList*>(_v) = _t->stopBitsOptions(); break;
        case 5: *reinterpret_cast< QVariantList*>(_v) = _t->flowControlOptions(); break;
        case 6: *reinterpret_cast< QVariantList*>(_v) = _t->lineEndingOptions(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->portName(); break;
        case 8: *reinterpret_cast< int*>(_v) = _t->baudRate(); break;
        case 9: *reinterpret_cast< int*>(_v) = _t->dataBits(); break;
        case 10: *reinterpret_cast< int*>(_v) = _t->parity(); break;
        case 11: *reinterpret_cast< int*>(_v) = _t->stopBits(); break;
        case 12: *reinterpret_cast< int*>(_v) = _t->flowControl(); break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->isOpen(); break;
        case 14: *reinterpret_cast< QString*>(_v) = _t->statusText(); break;
        case 15: *reinterpret_cast< QString*>(_v) = _t->errorText(); break;
        case 16: *reinterpret_cast< QString*>(_v) = _t->receivedText(); break;
        case 17: *reinterpret_cast< int*>(_v) = _t->lineEnding(); break;
        case 18: *reinterpret_cast< bool*>(_v) = _t->autoScroll(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 7: _t->setPortName(*reinterpret_cast< QString*>(_v)); break;
        case 8: _t->setBaudRate(*reinterpret_cast< int*>(_v)); break;
        case 9: _t->setDataBits(*reinterpret_cast< int*>(_v)); break;
        case 10: _t->setParity(*reinterpret_cast< int*>(_v)); break;
        case 11: _t->setStopBits(*reinterpret_cast< int*>(_v)); break;
        case 12: _t->setFlowControl(*reinterpret_cast< int*>(_v)); break;
        case 17: _t->setLineEnding(*reinterpret_cast< int*>(_v)); break;
        case 18: _t->setAutoScroll(*reinterpret_cast< bool*>(_v)); break;
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
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 24)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 24;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
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
void SerialPortController::receivedTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void SerialPortController::lineEndingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void SerialPortController::autoScrollChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void SerialPortController::sendSucceeded(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void SerialPortController::sendFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}
QT_WARNING_POP
