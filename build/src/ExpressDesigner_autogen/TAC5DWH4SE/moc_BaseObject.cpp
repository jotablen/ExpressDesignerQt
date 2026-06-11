/****************************************************************************
** Meta object code from reading C++ file 'BaseObject.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/BaseObject.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BaseObject.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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
struct qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t {};
} // unnamed namespace

template <> constexpr inline auto ExpressDesigner::BaseObject::qt_create_metaobjectdata<qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ExpressDesigner::BaseObject",
        "objectModified",
        "",
        "propertyName",
        "visibilityChanged",
        "visible",
        "nameChanged",
        "newName"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'objectModified'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'visibilityChanged'
        QtMocHelpers::SignalData<void(bool)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 5 },
        }}),
        // Signal 'nameChanged'
        QtMocHelpers::SignalData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<BaseObject, qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ExpressDesigner::BaseObject::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>.metaTypes,
    nullptr
} };

void ExpressDesigner::BaseObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BaseObject *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->objectModified((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->visibilityChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->nameChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BaseObject::*)(const QString & )>(_a, &BaseObject::objectModified, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BaseObject::*)(bool )>(_a, &BaseObject::visibilityChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BaseObject::*)(const QString & )>(_a, &BaseObject::nameChanged, 2))
            return;
    }
}

const QMetaObject *ExpressDesigner::BaseObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExpressDesigner::BaseObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner10BaseObjectE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ExpressDesigner::BaseObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void ExpressDesigner::BaseObject::objectModified(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ExpressDesigner::BaseObject::visibilityChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ExpressDesigner::BaseObject::nameChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
