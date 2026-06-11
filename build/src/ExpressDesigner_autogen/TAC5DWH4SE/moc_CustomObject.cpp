/****************************************************************************
** Meta object code from reading C++ file 'CustomObject.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/core/CustomObject.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CustomObject.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t {};
} // unnamed namespace

template <> constexpr inline auto ExpressDesigner::CustomObject::qt_create_metaobjectdata<qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ExpressDesigner::CustomObject",
        "refractiveIndexChanged",
        "",
        "index",
        "controlPointsChanged",
        "referencePointChanged",
        "QPointF",
        "point",
        "refractiveIndex"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'refractiveIndexChanged'
        QtMocHelpers::SignalData<void(double)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 3 },
        }}),
        // Signal 'controlPointsChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'referencePointChanged'
        QtMocHelpers::SignalData<void(const QPointF &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'refractiveIndex'
        QtMocHelpers::PropertyData<double>(8, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CustomObject, qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ExpressDesigner::CustomObject::staticMetaObject = { {
    QMetaObject::SuperData::link<BaseObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>.metaTypes,
    nullptr
} };

void ExpressDesigner::CustomObject::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CustomObject *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->refractiveIndexChanged((*reinterpret_cast<std::add_pointer_t<double>>(_a[1]))); break;
        case 1: _t->controlPointsChanged(); break;
        case 2: _t->referencePointChanged((*reinterpret_cast<std::add_pointer_t<QPointF>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CustomObject::*)(double )>(_a, &CustomObject::refractiveIndexChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CustomObject::*)()>(_a, &CustomObject::controlPointsChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CustomObject::*)(const QPointF & )>(_a, &CustomObject::referencePointChanged, 2))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<double*>(_v) = _t->refractiveIndex(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setRefractiveIndex(*reinterpret_cast<double*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *ExpressDesigner::CustomObject::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ExpressDesigner::CustomObject::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15ExpressDesigner12CustomObjectE_t>.strings))
        return static_cast<void*>(this);
    return BaseObject::qt_metacast(_clname);
}

int ExpressDesigner::CustomObject::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = BaseObject::qt_metacall(_c, _id, _a);
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
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ExpressDesigner::CustomObject::refractiveIndexChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ExpressDesigner::CustomObject::controlPointsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ExpressDesigner::CustomObject::referencePointChanged(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
