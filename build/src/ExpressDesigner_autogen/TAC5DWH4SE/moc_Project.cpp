/****************************************************************************
** Meta object code from reading C++ file 'Project.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/Project.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Project.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15ExpressDesigner7ProjectE_t {};
} // unnamed namespace

template <> constexpr inline auto ExpressDesigner::Project::qt_create_metaobjectdata<qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ExpressDesigner::Project",
        "dataObjectAdded",
        "",
        "CustomObject*",
        "obj",
        "index",
        "dataObjectRemoved",
        "resultObjectAdded",
        "resultObjectRemoved",
        "operationAdded",
        "CustomOperation*",
        "op",
        "operationRemoved",
        "scaleChanged",
        "projectCleared"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'dataObjectAdded'
        QtMocHelpers::SignalData<void(CustomObject *, int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'dataObjectRemoved'
        QtMocHelpers::SignalData<void(CustomObject *, int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'resultObjectAdded'
        QtMocHelpers::SignalData<void(CustomObject *, int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'resultObjectRemoved'
        QtMocHelpers::SignalData<void(CustomObject *, int)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'operationAdded'
        QtMocHelpers::SignalData<void(CustomOperation *, int)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'operationRemoved'
        QtMocHelpers::SignalData<void(CustomOperation *, int)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 10, 11 }, { QMetaType::Int, 5 },
        }}),
        // Signal 'scaleChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'projectCleared'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Project, qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ExpressDesigner::Project::staticMetaObject = { {
    QMetaObject::SuperData::link<BaseObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>.metaTypes,
    nullptr
} };

void ExpressDesigner::Project::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Project *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->dataObjectAdded((*reinterpret_cast<std::add_pointer_t<CustomObject*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->dataObjectRemoved((*reinterpret_cast<std::add_pointer_t<CustomObject*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->resultObjectAdded((*reinterpret_cast<std::add_pointer_t<CustomObject*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->resultObjectRemoved((*reinterpret_cast<std::add_pointer_t<CustomObject*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->operationAdded((*reinterpret_cast<std::add_pointer_t<CustomOperation*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->operationRemoved((*reinterpret_cast<std::add_pointer_t<CustomOperation*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 6: _t->scaleChanged(); break;
        case 7: _t->projectCleared(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomObject* >(); break;
            }
            break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomObject* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomObject* >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomObject* >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomOperation* >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< CustomOperation* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomObject * , int )>(_a, &Project::dataObjectAdded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomObject * , int )>(_a, &Project::dataObjectRemoved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomObject * , int )>(_a, &Project::resultObjectAdded, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomObject * , int )>(_a, &Project::resultObjectRemoved, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomOperation * , int )>(_a, &Project::operationAdded, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)(CustomOperation * , int )>(_a, &Project::operationRemoved, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)()>(_a, &Project::scaleChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (Project::*)()>(_a, &Project::projectCleared, 7))
            return;
    }
}

const QMetaObject *ExpressDesigner::Project::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExpressDesigner::Project::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner7ProjectE_t>.strings))
        return static_cast<void*>(this);
    return BaseObject::qt_metacast(_clname);
}

int ExpressDesigner::Project::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BaseObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void ExpressDesigner::Project::dataObjectAdded(CustomObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void ExpressDesigner::Project::dataObjectRemoved(CustomObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void ExpressDesigner::Project::resultObjectAdded(CustomObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void ExpressDesigner::Project::resultObjectRemoved(CustomObject * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}

// SIGNAL 4
void ExpressDesigner::Project::operationAdded(CustomOperation * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void ExpressDesigner::Project::operationRemoved(CustomOperation * _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2);
}

// SIGNAL 6
void ExpressDesigner::Project::scaleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void ExpressDesigner::Project::projectCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
