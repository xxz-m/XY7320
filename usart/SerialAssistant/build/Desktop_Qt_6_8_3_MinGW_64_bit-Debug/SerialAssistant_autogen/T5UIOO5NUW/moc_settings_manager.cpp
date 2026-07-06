/****************************************************************************
** Meta object code from reading C++ file 'settings_manager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend/settings/settings_manager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'settings_manager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15SettingsManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN15SettingsManagerE = QtMocHelpers::stringData(
    "SettingsManager",
    "serialSettingsChanged",
    "",
    "autoScrollChanged",
    "hexDisplayChanged",
    "timestampEnabledChanged",
    "timestampFormatChanged",
    "themeModeChanged",
    "accentColorChanged",
    "uiDensityChanged",
    "terminalFontChanged",
    "transmitSettingsChanged",
    "restoreDefaults",
    "autoScroll",
    "hexDisplay",
    "timestampEnabled",
    "timestampFormat",
    "themeMode",
    "accentColor",
    "uiDensity",
    "terminalFont"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN15SettingsManagerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       8,   91, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    9 /* Public */,
       3,    0,   81,    2, 0x06,   10 /* Public */,
       4,    0,   82,    2, 0x06,   11 /* Public */,
       5,    0,   83,    2, 0x06,   12 /* Public */,
       6,    0,   84,    2, 0x06,   13 /* Public */,
       7,    0,   85,    2, 0x06,   14 /* Public */,
       8,    0,   86,    2, 0x06,   15 /* Public */,
       9,    0,   87,    2, 0x06,   16 /* Public */,
      10,    0,   88,    2, 0x06,   17 /* Public */,
      11,    0,   89,    2, 0x06,   18 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    0,   90,    2, 0x0a,   19 /* Public */,

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
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
      13, QMetaType::Bool, 0x00015103, uint(1), 0,
      14, QMetaType::Bool, 0x00015103, uint(2), 0,
      15, QMetaType::Bool, 0x00015103, uint(3), 0,
      16, QMetaType::QString, 0x00015103, uint(4), 0,
      17, QMetaType::Int, 0x00015103, uint(5), 0,
      18, QMetaType::QString, 0x00015103, uint(6), 0,
      19, QMetaType::Int, 0x00015103, uint(7), 0,
      20, QMetaType::QString, 0x00015103, uint(8), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SettingsManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN15SettingsManagerE.offsetsAndSizes,
    qt_meta_data_ZN15SettingsManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN15SettingsManagerE_t,
        // property 'autoScroll'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'hexDisplay'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'timestampEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'timestampFormat'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'themeMode'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'accentColor'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'uiDensity'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'terminalFont'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SettingsManager, std::true_type>,
        // method 'serialSettingsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'autoScrollChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hexDisplayChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'timestampEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'timestampFormatChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'themeModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'accentColorChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'uiDensityChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'terminalFontChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'transmitSettingsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'restoreDefaults'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void SettingsManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SettingsManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->serialSettingsChanged(); break;
        case 1: _t->autoScrollChanged(); break;
        case 2: _t->hexDisplayChanged(); break;
        case 3: _t->timestampEnabledChanged(); break;
        case 4: _t->timestampFormatChanged(); break;
        case 5: _t->themeModeChanged(); break;
        case 6: _t->accentColorChanged(); break;
        case 7: _t->uiDensityChanged(); break;
        case 8: _t->terminalFontChanged(); break;
        case 9: _t->transmitSettingsChanged(); break;
        case 10: _t->restoreDefaults(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::serialSettingsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::autoScrollChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::hexDisplayChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::timestampEnabledChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::timestampFormatChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::themeModeChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::accentColorChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::uiDensityChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::terminalFontChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (SettingsManager::*)();
            if (_q_method_type _q_method = &SettingsManager::transmitSettingsChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->autoScroll(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->hexDisplay(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->timestampEnabled(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->timestampFormat(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->themeMode(); break;
        case 5: *reinterpret_cast< QString*>(_v) = _t->accentColor(); break;
        case 6: *reinterpret_cast< int*>(_v) = _t->uiDensity(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->terminalFont(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setAutoScroll(*reinterpret_cast< bool*>(_v)); break;
        case 1: _t->setHexDisplay(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->setTimestampEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setTimestampFormat(*reinterpret_cast< QString*>(_v)); break;
        case 4: _t->setThemeMode(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->setAccentColor(*reinterpret_cast< QString*>(_v)); break;
        case 6: _t->setUiDensity(*reinterpret_cast< int*>(_v)); break;
        case 7: _t->setTerminalFont(*reinterpret_cast< QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SettingsManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN15SettingsManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SettingsManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void SettingsManager::serialSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SettingsManager::autoScrollChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SettingsManager::hexDisplayChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SettingsManager::timestampEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SettingsManager::timestampFormatChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SettingsManager::themeModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void SettingsManager::accentColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void SettingsManager::uiDensityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void SettingsManager::terminalFontChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void SettingsManager::transmitSettingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}
QT_WARNING_POP
