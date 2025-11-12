/****************************************************************************
** Meta object code from reading C++ file 'finddialog.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/finddialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'finddialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
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
struct qt_meta_tag_ZN10FindDialogE_t {};
} // unnamed namespace

template <> constexpr inline auto FindDialog::qt_create_metaobjectdata<qt_meta_tag_ZN10FindDialogE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "FindDialog",
        "findRequested",
        "",
        "text",
        "forward",
        "caseSensitive",
        "wholeWords",
        "useRegex",
        "replaceRequested",
        "findText",
        "replaceText",
        "replaceAllRequested",
        "findNext",
        "findPrevious",
        "replace",
        "replaceAll",
        "onFindTextChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'findRequested'
        QtMocHelpers::SignalData<void(const QString &, bool, bool, bool, bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 4 }, { QMetaType::Bool, 5 }, { QMetaType::Bool, 6 },
            { QMetaType::Bool, 7 },
        }}),
        // Signal 'replaceRequested'
        QtMocHelpers::SignalData<void(const QString &, const QString &, bool, bool, bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::QString, 10 }, { QMetaType::Bool, 5 }, { QMetaType::Bool, 6 },
            { QMetaType::Bool, 7 },
        }}),
        // Signal 'replaceAllRequested'
        QtMocHelpers::SignalData<void(const QString &, const QString &, bool, bool, bool)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::QString, 10 }, { QMetaType::Bool, 5 }, { QMetaType::Bool, 6 },
            { QMetaType::Bool, 7 },
        }}),
        // Slot 'findNext'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'findPrevious'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'replace'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'replaceAll'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onFindTextChanged'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<FindDialog, qt_meta_tag_ZN10FindDialogE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject FindDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FindDialogE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FindDialogE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10FindDialogE_t>.metaTypes,
    nullptr
} };

void FindDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<FindDialog *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->findRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 1: _t->replaceRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 2: _t->replaceAllRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 3: _t->findNext(); break;
        case 4: _t->findPrevious(); break;
        case 5: _t->replace(); break;
        case 6: _t->replaceAll(); break;
        case 7: _t->onFindTextChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (FindDialog::*)(const QString & , bool , bool , bool , bool )>(_a, &FindDialog::findRequested, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (FindDialog::*)(const QString & , const QString & , bool , bool , bool )>(_a, &FindDialog::replaceRequested, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (FindDialog::*)(const QString & , const QString & , bool , bool , bool )>(_a, &FindDialog::replaceAllRequested, 2))
            return;
    }
}

const QMetaObject *FindDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FindDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10FindDialogE_t>.strings))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FindDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void FindDialog::findRequested(const QString & _t1, bool _t2, bool _t3, bool _t4, bool _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 1
void FindDialog::replaceRequested(const QString & _t1, const QString & _t2, bool _t3, bool _t4, bool _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 2
void FindDialog::replaceAllRequested(const QString & _t1, const QString & _t2, bool _t3, bool _t4, bool _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3, _t4, _t5);
}
QT_WARNING_POP
