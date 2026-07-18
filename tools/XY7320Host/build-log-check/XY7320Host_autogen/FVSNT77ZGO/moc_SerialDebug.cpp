/****************************************************************************
** Meta object code from reading C++ file 'SerialDebug.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/SerialDebug.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SerialDebug.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11SerialDebugE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN11SerialDebugE = QtMocHelpers::stringData(
    "SerialDebug",
    "portsChanged",
    "",
    "portNameChanged",
    "baudRateChanged",
    "isOpenChanged",
    "logTextChanged",
    "autoScrollChanged",
    "showHexChanged",
    "rxBytesChanged",
    "txBytesChanged",
    "frameChanged",
    "handleReadyRead",
    "data",
    "handleWriteFinished",
    "totalBytes",
    "tag",
    "handleError",
    "error",
    "message",
    "refreshPorts",
    "open",
    "close",
    "send",
    "sendHex",
    "hex",
    "clear",
    "switchMode",
    "mode",
    "ports",
    "QVariantList",
    "portName",
    "baudRate",
    "isOpen",
    "logText",
    "autoScroll",
    "showHex",
    "rxBytes",
    "txBytes",
    "frameStatus",
    "frameDirection",
    "frameModel",
    "frameCommand",
    "frameLength",
    "framePayloadHex",
    "frameCrcStatus",
    "businessType",
    "businessFields"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN11SerialDebugE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
      18,  170, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  134,    2, 0x06,   19 /* Public */,
       3,    0,  135,    2, 0x06,   20 /* Public */,
       4,    0,  136,    2, 0x06,   21 /* Public */,
       5,    0,  137,    2, 0x06,   22 /* Public */,
       6,    0,  138,    2, 0x06,   23 /* Public */,
       7,    0,  139,    2, 0x06,   24 /* Public */,
       8,    0,  140,    2, 0x06,   25 /* Public */,
       9,    0,  141,    2, 0x06,   26 /* Public */,
      10,    0,  142,    2, 0x06,   27 /* Public */,
      11,    0,  143,    2, 0x06,   28 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    1,  144,    2, 0x08,   29 /* Private */,
      14,    2,  147,    2, 0x08,   31 /* Private */,
      17,    2,  152,    2, 0x08,   34 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      20,    0,  157,    2, 0x02,   37 /* Public */,
      21,    0,  158,    2, 0x02,   38 /* Public */,
      22,    0,  159,    2, 0x02,   39 /* Public */,
      23,    1,  160,    2, 0x02,   40 /* Public */,
      24,    1,  163,    2, 0x02,   42 /* Public */,
      26,    0,  166,    2, 0x02,   44 /* Public */,
      27,    1,  167,    2, 0x02,   45 /* Public */,

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

 // slots: parameters
    QMetaType::Void, QMetaType::QByteArray,   13,
    QMetaType::Void, QMetaType::LongLong, QMetaType::Int,   15,   16,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   18,   19,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   28,

 // properties: name, type, flags, notifyId, revision
      29, 0x80000000 | 30, 0x00015009, uint(0), 0,
      31, QMetaType::QString, 0x00015103, uint(1), 0,
      32, QMetaType::Int, 0x00015103, uint(2), 0,
      33, QMetaType::Bool, 0x00015001, uint(3), 0,
      34, QMetaType::QString, 0x00015001, uint(4), 0,
      35, QMetaType::Bool, 0x00015103, uint(5), 0,
      36, QMetaType::Bool, 0x00015103, uint(6), 0,
      37, QMetaType::LongLong, 0x00015001, uint(7), 0,
      38, QMetaType::LongLong, 0x00015001, uint(8), 0,
      39, QMetaType::QString, 0x00015001, uint(9), 0,
      40, QMetaType::QString, 0x00015001, uint(9), 0,
      41, QMetaType::QString, 0x00015001, uint(9), 0,
      42, QMetaType::QString, 0x00015001, uint(9), 0,
      43, QMetaType::QString, 0x00015001, uint(9), 0,
      44, QMetaType::QString, 0x00015001, uint(9), 0,
      45, QMetaType::QString, 0x00015001, uint(9), 0,
      46, QMetaType::QString, 0x00015001, uint(9), 0,
      47, 0x80000000 | 30, 0x00015009, uint(9), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SerialDebug::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN11SerialDebugE.offsetsAndSizes,
    qt_meta_data_ZN11SerialDebugE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN11SerialDebugE_t,
        // property 'ports'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'portName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'baudRate'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'isOpen'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'logText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'autoScroll'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'showHex'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'rxBytes'
        QtPrivate::TypeAndForceComplete<qint64, std::true_type>,
        // property 'txBytes'
        QtPrivate::TypeAndForceComplete<qint64, std::true_type>,
        // property 'frameStatus'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'frameDirection'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'frameModel'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'frameCommand'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'frameLength'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'framePayloadHex'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'frameCrcStatus'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'businessType'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'businessFields'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SerialDebug, std::true_type>,
        // method 'portsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'portNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'baudRateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'isOpenChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'logTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'autoScrollChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'showHexChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'rxBytesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'txBytesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'frameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'handleWriteFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'refreshPorts'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'open'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'close'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'send'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'sendHex'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'clear'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'switchMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void SerialDebug::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SerialDebug *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portsChanged(); break;
        case 1: _t->portNameChanged(); break;
        case 2: _t->baudRateChanged(); break;
        case 3: _t->isOpenChanged(); break;
        case 4: _t->logTextChanged(); break;
        case 5: _t->autoScrollChanged(); break;
        case 6: _t->showHexChanged(); break;
        case 7: _t->rxBytesChanged(); break;
        case 8: _t->txBytesChanged(); break;
        case 9: _t->frameChanged(); break;
        case 10: _t->handleReadyRead((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 11: _t->handleWriteFinished((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 12: _t->handleError((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 13: _t->refreshPorts(); break;
        case 14: _t->open(); break;
        case 15: _t->close(); break;
        case 16: _t->send((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->sendHex((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 18: _t->clear(); break;
        case 19: _t->switchMode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::portsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::portNameChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::baudRateChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::isOpenChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::logTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::autoScrollChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::showHexChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::rxBytesChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::txBytesChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (SerialDebug::*)();
            if (_q_method_type _q_method = &SerialDebug::frameChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
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
        case 4: *reinterpret_cast< QString*>(_v) = _t->logText(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->autoScroll(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->showHex(); break;
        case 7: *reinterpret_cast< qint64*>(_v) = _t->rxBytes(); break;
        case 8: *reinterpret_cast< qint64*>(_v) = _t->txBytes(); break;
        case 9: *reinterpret_cast< QString*>(_v) = _t->frameStatus(); break;
        case 10: *reinterpret_cast< QString*>(_v) = _t->frameDirection(); break;
        case 11: *reinterpret_cast< QString*>(_v) = _t->frameModel(); break;
        case 12: *reinterpret_cast< QString*>(_v) = _t->frameCommand(); break;
        case 13: *reinterpret_cast< QString*>(_v) = _t->frameLength(); break;
        case 14: *reinterpret_cast< QString*>(_v) = _t->framePayloadHex(); break;
        case 15: *reinterpret_cast< QString*>(_v) = _t->frameCrcStatus(); break;
        case 16: *reinterpret_cast< QString*>(_v) = _t->businessType(); break;
        case 17: *reinterpret_cast< QVariantList*>(_v) = _t->businessFields(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setPortName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->setBaudRate(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->setAutoScroll(*reinterpret_cast< bool*>(_v)); break;
        case 6: _t->setShowHex(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SerialDebug::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SerialDebug::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN11SerialDebugE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SerialDebug::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void SerialDebug::portsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SerialDebug::portNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SerialDebug::baudRateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SerialDebug::isOpenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SerialDebug::logTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SerialDebug::autoScrollChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SerialDebug::showHexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SerialDebug::rxBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SerialDebug::txBytesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void SerialDebug::frameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
