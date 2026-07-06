/****************************************************************************
** Meta object code from reading C++ file 'transmit_controller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/console/transmit_controller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'transmit_controller.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN18TransmitControllerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN18TransmitControllerE = QtMocHelpers::stringData(
    "TransmitController",
    "hexSendChanged",
    "",
    "loopSendChanged",
    "loopIntervalMsChanged",
    "lineEndingChanged",
    "statisticsChanged",
    "errorTextChanged",
    "sendLoopPayload",
    "handleSerialError",
    "message",
    "send",
    "input",
    "startLoopSend",
    "stopLoopSend",
    "hexSend",
    "loopSend",
    "loopIntervalMs",
    "lineEnding",
    "txBytes",
    "txFrames",
    "errorText",
    "LineEnding",
    "None",
    "LF",
    "CR",
    "CRLF"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN18TransmitControllerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       7,   97, // properties
       1,  132, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    9 /* Public */,
       3,    0,   81,    2, 0x06,   10 /* Public */,
       4,    0,   82,    2, 0x06,   11 /* Public */,
       5,    0,   83,    2, 0x06,   12 /* Public */,
       6,    0,   84,    2, 0x06,   13 /* Public */,
       7,    0,   85,    2, 0x06,   14 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    0,   86,    2, 0x08,   15 /* Private */,
       9,    1,   87,    2, 0x08,   16 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,   90,    2, 0x02,   18 /* Public */,
      13,    1,   93,    2, 0x02,   20 /* Public */,
      14,    0,   96,    2, 0x02,   22 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   10,

 // methods: parameters
    QMetaType::Bool, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      15, QMetaType::Bool, 0x00015103, uint(0), 0,
      16, QMetaType::Bool, 0x00015103, uint(1), 0,
      17, QMetaType::Int, 0x00015103, uint(2), 0,
      18, QMetaType::Int, 0x00015103, uint(3), 0,
      19, QMetaType::ULongLong, 0x00015001, uint(4), 0,
      20, QMetaType::ULongLong, 0x00015001, uint(4), 0,
      21, QMetaType::QString, 0x00015001, uint(5), 0,

 // enums: name, alias, flags, count, data
      22,   22, 0x0,    4,  137,

 // enum data: key, value
      23, uint(TransmitController::None),
      24, uint(TransmitController::LF),
      25, uint(TransmitController::CR),
      26, uint(TransmitController::CRLF),

       0        // eod
};

Q_CONSTINIT const QMetaObject TransmitController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN18TransmitControllerE.offsetsAndSizes,
    qt_meta_data_ZN18TransmitControllerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN18TransmitControllerE_t,
        // property 'hexSend'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'loopSend'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'loopIntervalMs'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'lineEnding'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'txBytes'
        QtPrivate::TypeAndForceComplete<qulonglong, std::true_type>,
        // property 'txFrames'
        QtPrivate::TypeAndForceComplete<qulonglong, std::true_type>,
        // property 'errorText'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // enum 'LineEnding'
        QtPrivate::TypeAndForceComplete<TransmitController::LineEnding, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TransmitController, std::true_type>,
        // method 'hexSendChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'loopSendChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'loopIntervalMsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'lineEndingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'statisticsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'errorTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendLoopPayload'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSerialError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'send'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'startLoopSend'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'stopLoopSend'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TransmitController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TransmitController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->hexSendChanged(); break;
        case 1: _t->loopSendChanged(); break;
        case 2: _t->loopIntervalMsChanged(); break;
        case 3: _t->lineEndingChanged(); break;
        case 4: _t->statisticsChanged(); break;
        case 5: _t->errorTextChanged(); break;
        case 6: _t->sendLoopPayload(); break;
        case 7: _t->handleSerialError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: { bool _r = _t->send((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->startLoopSend((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->stopLoopSend(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::hexSendChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::loopSendChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::loopIntervalMsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::lineEndingChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::statisticsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (TransmitController::*)();
            if (_q_method_type _q_method = &TransmitController::errorTextChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->hexSend(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->loopSend(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->loopIntervalMs(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->lineEnding(); break;
        case 4: *reinterpret_cast< qulonglong*>(_v) = _t->txBytes(); break;
        case 5: *reinterpret_cast< qulonglong*>(_v) = _t->txFrames(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->errorText(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setHexSend(*reinterpret_cast< bool*>(_v)); break;
        case 1: _t->setLoopSend(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->setLoopIntervalMs(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->setLineEnding(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *TransmitController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TransmitController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN18TransmitControllerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TransmitController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void TransmitController::hexSendChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TransmitController::loopSendChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TransmitController::loopIntervalMsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TransmitController::lineEndingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TransmitController::statisticsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TransmitController::errorTextChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
