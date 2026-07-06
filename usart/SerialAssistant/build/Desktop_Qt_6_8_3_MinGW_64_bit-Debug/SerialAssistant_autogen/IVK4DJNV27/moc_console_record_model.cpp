/****************************************************************************
** Meta object code from reading C++ file 'console_record_model.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../models/console_record_model.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'console_record_model.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN18ConsoleRecordModelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN18ConsoleRecordModelE = QtMocHelpers::stringData(
    "ConsoleRecordModel",
    "clear",
    "",
    "Role",
    "IdRole",
    "RecordTimeRole",
    "DirectionRole",
    "ContentRole",
    "TextRole",
    "HexTextRole",
    "ByteCountRole",
    "IsErrorRole",
    "DirectionColorRole"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN18ConsoleRecordModelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       1,   21, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x02,    2 /* Public */,

 // methods: parameters
    QMetaType::Void,

 // enums: name, alias, flags, count, data
       3,    3, 0x0,    9,   26,

 // enum data: key, value
       4, uint(ConsoleRecordModel::IdRole),
       5, uint(ConsoleRecordModel::RecordTimeRole),
       6, uint(ConsoleRecordModel::DirectionRole),
       7, uint(ConsoleRecordModel::ContentRole),
       8, uint(ConsoleRecordModel::TextRole),
       9, uint(ConsoleRecordModel::HexTextRole),
      10, uint(ConsoleRecordModel::ByteCountRole),
      11, uint(ConsoleRecordModel::IsErrorRole),
      12, uint(ConsoleRecordModel::DirectionColorRole),

       0        // eod
};

Q_CONSTINIT const QMetaObject ConsoleRecordModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ZN18ConsoleRecordModelE.offsetsAndSizes,
    qt_meta_data_ZN18ConsoleRecordModelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN18ConsoleRecordModelE_t,
        // enum 'Role'
        QtPrivate::TypeAndForceComplete<ConsoleRecordModel::Role, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ConsoleRecordModel, std::true_type>,
        // method 'clear'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ConsoleRecordModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ConsoleRecordModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->clear(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *ConsoleRecordModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ConsoleRecordModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN18ConsoleRecordModelE.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int ConsoleRecordModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
