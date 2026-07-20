/****************************************************************************
** Meta object code from reading C++ file 'usbmanager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../backend/usbmanager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'usbmanager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN10UsbManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN10UsbManagerE = QtMocHelpers::stringData(
    "UsbManager",
    "drivesChanged",
    "",
    "filesChanged",
    "selectedDriveChanged",
    "usbConnectedChanged",
    "checkDrives",
    "refreshDrives",
    "selectDrive",
    "path",
    "drives",
    "QVariantList",
    "files",
    "selectedDrive",
    "usbConnected"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN10UsbManagerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       4,   65, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    5 /* Public */,
       3,    0,   57,    2, 0x06,    6 /* Public */,
       4,    0,   58,    2, 0x06,    7 /* Public */,
       5,    0,   59,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   60,    2, 0x08,    9 /* Private */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   61,    2, 0x02,   10 /* Public */,
       8,    1,   62,    2, 0x02,   11 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,

 // properties: name, type, flags, notifyId, revision
      10, 0x80000000 | 11, 0x00015009, uint(0), 0,
      12, 0x80000000 | 11, 0x00015009, uint(1), 0,
      13, QMetaType::QString, 0x00015001, uint(2), 0,
      14, QMetaType::Bool, 0x00015001, uint(3), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject UsbManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN10UsbManagerE.offsetsAndSizes,
    qt_meta_data_ZN10UsbManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN10UsbManagerE_t,
        // property 'drives'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'files'
        QtPrivate::TypeAndForceComplete<QVariantList, std::true_type>,
        // property 'selectedDrive'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'usbConnected'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<UsbManager, std::true_type>,
        // method 'drivesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'filesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'selectedDriveChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'usbConnectedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'checkDrives'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'refreshDrives'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'selectDrive'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void UsbManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<UsbManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->drivesChanged(); break;
        case 1: _t->filesChanged(); break;
        case 2: _t->selectedDriveChanged(); break;
        case 3: _t->usbConnectedChanged(); break;
        case 4: _t->checkDrives(); break;
        case 5: _t->refreshDrives(); break;
        case 6: _t->selectDrive((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (UsbManager::*)();
            if (_q_method_type _q_method = &UsbManager::drivesChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (UsbManager::*)();
            if (_q_method_type _q_method = &UsbManager::filesChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (UsbManager::*)();
            if (_q_method_type _q_method = &UsbManager::selectedDriveChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (UsbManager::*)();
            if (_q_method_type _q_method = &UsbManager::usbConnectedChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QVariantList*>(_v) = _t->drives(); break;
        case 1: *reinterpret_cast< QVariantList*>(_v) = _t->files(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->selectedDrive(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->usbConnected(); break;
        default: break;
        }
    }
}

const QMetaObject *UsbManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UsbManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN10UsbManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int UsbManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
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
void UsbManager::drivesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void UsbManager::filesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void UsbManager::selectedDriveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void UsbManager::usbConnectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
