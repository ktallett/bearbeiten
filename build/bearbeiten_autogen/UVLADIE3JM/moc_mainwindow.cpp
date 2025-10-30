/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/mainwindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
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
struct qt_meta_tag_ZN10MainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN10MainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MainWindow",
        "newFile",
        "",
        "openFile",
        "saveFile",
        "saveAsFile",
        "exitApp",
        "undo",
        "redo",
        "cut",
        "copy",
        "paste",
        "showFindDialog",
        "showReplaceDialog",
        "showGoToLineDialog",
        "showSymbolSearchDialog",
        "onLanguageChanged",
        "index",
        "onTabChanged",
        "onTabCloseRequested",
        "onTabContextMenu",
        "pos",
        "toggleSplitView",
        "splitHorizontally",
        "splitVertically",
        "closeSplitView",
        "onSplitterFocusChanged",
        "toggleProjectPanel",
        "openProjectFromPanel",
        "filePath",
        "toggleOutlinePanel",
        "jumpToSymbolFromOutline",
        "lineNumber",
        "updateOutlinePanel",
        "autoSave",
        "onTextChanged",
        "toggleAutoSave",
        "toggleTheme",
        "toggleLineWrap",
        "setLineWrapMode",
        "mode",
        "toggleWordWrapMode",
        "toggleColumnRuler",
        "toggleWrapIndicator",
        "setWrapColumn",
        "foldCurrentBlock",
        "unfoldCurrentBlock",
        "foldAll",
        "unfoldAll",
        "toggleMinimap",
        "toggleIndentationGuides",
        "toggleActiveIndentHighlight",
        "performFind",
        "text",
        "forward",
        "caseSensitive",
        "wholeWords",
        "useRegex",
        "performReplace",
        "findText",
        "replaceText",
        "performReplaceAll",
        "performGoToLine",
        "updateLinePreview",
        "performSymbolJump",
        "updateBreadcrumb",
        "updateBreadcrumbSymbol",
        "showCharacterInspector",
        "changeEncoding",
        "onEncodingLabelClicked",
        "toggleBookmark",
        "goToNextBookmark",
        "goToPreviousBookmark",
        "clearAllBookmarks",
        "duplicateLine",
        "deleteLine",
        "moveLineUp",
        "moveLineDown",
        "sortLinesAscending",
        "sortLinesDescending"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'newFile'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openFile'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'saveFile'
        QtMocHelpers::SlotData<bool()>(4, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'saveAsFile'
        QtMocHelpers::SlotData<bool()>(5, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'exitApp'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'undo'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'redo'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'cut'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'copy'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'paste'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showFindDialog'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showReplaceDialog'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showGoToLineDialog'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showSymbolSearchDialog'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onLanguageChanged'
        QtMocHelpers::SlotData<void(int)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 17 },
        }}),
        // Slot 'onTabChanged'
        QtMocHelpers::SlotData<void(int)>(18, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 17 },
        }}),
        // Slot 'onTabCloseRequested'
        QtMocHelpers::SlotData<void(int)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 17 },
        }}),
        // Slot 'onTabContextMenu'
        QtMocHelpers::SlotData<void(const QPoint &)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPoint, 21 },
        }}),
        // Slot 'toggleSplitView'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'splitHorizontally'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'splitVertically'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'closeSplitView'
        QtMocHelpers::SlotData<void()>(25, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onSplitterFocusChanged'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleProjectPanel'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'openProjectFromPanel'
        QtMocHelpers::SlotData<void(const QString &)>(28, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Slot 'toggleOutlinePanel'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'jumpToSymbolFromOutline'
        QtMocHelpers::SlotData<void(int)>(31, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'updateOutlinePanel'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'autoSave'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onTextChanged'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleAutoSave'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleTheme'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleLineWrap'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setLineWrapMode'
        QtMocHelpers::SlotData<void(int)>(39, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 40 },
        }}),
        // Slot 'toggleWordWrapMode'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleColumnRuler'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleWrapIndicator'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'setWrapColumn'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'foldCurrentBlock'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'unfoldCurrentBlock'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'foldAll'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'unfoldAll'
        QtMocHelpers::SlotData<void()>(48, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleMinimap'
        QtMocHelpers::SlotData<void()>(49, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleIndentationGuides'
        QtMocHelpers::SlotData<void()>(50, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleActiveIndentHighlight'
        QtMocHelpers::SlotData<void()>(51, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'performFind'
        QtMocHelpers::SlotData<void(const QString &, bool, bool, bool, bool)>(52, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 53 }, { QMetaType::Bool, 54 }, { QMetaType::Bool, 55 }, { QMetaType::Bool, 56 },
            { QMetaType::Bool, 57 },
        }}),
        // Slot 'performReplace'
        QtMocHelpers::SlotData<void(const QString &, const QString &, bool, bool, bool)>(58, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 59 }, { QMetaType::QString, 60 }, { QMetaType::Bool, 55 }, { QMetaType::Bool, 56 },
            { QMetaType::Bool, 57 },
        }}),
        // Slot 'performReplaceAll'
        QtMocHelpers::SlotData<void(const QString &, const QString &, bool, bool, bool)>(61, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 59 }, { QMetaType::QString, 60 }, { QMetaType::Bool, 55 }, { QMetaType::Bool, 56 },
            { QMetaType::Bool, 57 },
        }}),
        // Slot 'performGoToLine'
        QtMocHelpers::SlotData<void(int)>(62, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'updateLinePreview'
        QtMocHelpers::SlotData<void(int)>(63, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'performSymbolJump'
        QtMocHelpers::SlotData<void(int)>(64, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 32 },
        }}),
        // Slot 'updateBreadcrumb'
        QtMocHelpers::SlotData<void()>(65, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'updateBreadcrumbSymbol'
        QtMocHelpers::SlotData<void()>(66, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showCharacterInspector'
        QtMocHelpers::SlotData<void()>(67, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'changeEncoding'
        QtMocHelpers::SlotData<void()>(68, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onEncodingLabelClicked'
        QtMocHelpers::SlotData<void()>(69, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleBookmark'
        QtMocHelpers::SlotData<void()>(70, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'goToNextBookmark'
        QtMocHelpers::SlotData<void()>(71, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'goToPreviousBookmark'
        QtMocHelpers::SlotData<void()>(72, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'clearAllBookmarks'
        QtMocHelpers::SlotData<void()>(73, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'duplicateLine'
        QtMocHelpers::SlotData<void()>(74, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'deleteLine'
        QtMocHelpers::SlotData<void()>(75, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'moveLineUp'
        QtMocHelpers::SlotData<void()>(76, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'moveLineDown'
        QtMocHelpers::SlotData<void()>(77, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sortLinesAscending'
        QtMocHelpers::SlotData<void()>(78, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'sortLinesDescending'
        QtMocHelpers::SlotData<void()>(79, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MainWindow, qt_meta_tag_ZN10MainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10MainWindowE_t>.metaTypes,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->newFile(); break;
        case 1: _t->openFile(); break;
        case 2: { bool _r = _t->saveFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->saveAsFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 4: _t->exitApp(); break;
        case 5: _t->undo(); break;
        case 6: _t->redo(); break;
        case 7: _t->cut(); break;
        case 8: _t->copy(); break;
        case 9: _t->paste(); break;
        case 10: _t->showFindDialog(); break;
        case 11: _t->showReplaceDialog(); break;
        case 12: _t->showGoToLineDialog(); break;
        case 13: _t->showSymbolSearchDialog(); break;
        case 14: _t->onLanguageChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->onTabCloseRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onTabContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 18: _t->toggleSplitView(); break;
        case 19: _t->splitHorizontally(); break;
        case 20: _t->splitVertically(); break;
        case 21: _t->closeSplitView(); break;
        case 22: _t->onSplitterFocusChanged(); break;
        case 23: _t->toggleProjectPanel(); break;
        case 24: _t->openProjectFromPanel((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 25: _t->toggleOutlinePanel(); break;
        case 26: _t->jumpToSymbolFromOutline((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 27: _t->updateOutlinePanel(); break;
        case 28: _t->autoSave(); break;
        case 29: _t->onTextChanged(); break;
        case 30: _t->toggleAutoSave(); break;
        case 31: _t->toggleTheme(); break;
        case 32: _t->toggleLineWrap(); break;
        case 33: _t->setLineWrapMode((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 34: _t->toggleWordWrapMode(); break;
        case 35: _t->toggleColumnRuler(); break;
        case 36: _t->toggleWrapIndicator(); break;
        case 37: _t->setWrapColumn(); break;
        case 38: _t->foldCurrentBlock(); break;
        case 39: _t->unfoldCurrentBlock(); break;
        case 40: _t->foldAll(); break;
        case 41: _t->unfoldAll(); break;
        case 42: _t->toggleMinimap(); break;
        case 43: _t->toggleIndentationGuides(); break;
        case 44: _t->toggleActiveIndentHighlight(); break;
        case 45: _t->performFind((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 46: _t->performReplace((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 47: _t->performReplaceAll((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5]))); break;
        case 48: _t->performGoToLine((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 49: _t->updateLinePreview((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 50: _t->performSymbolJump((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 51: _t->updateBreadcrumb(); break;
        case 52: _t->updateBreadcrumbSymbol(); break;
        case 53: _t->showCharacterInspector(); break;
        case 54: _t->changeEncoding(); break;
        case 55: _t->onEncodingLabelClicked(); break;
        case 56: _t->toggleBookmark(); break;
        case 57: _t->goToNextBookmark(); break;
        case 58: _t->goToPreviousBookmark(); break;
        case 59: _t->clearAllBookmarks(); break;
        case 60: _t->duplicateLine(); break;
        case 61: _t->deleteLine(); break;
        case 62: _t->moveLineUp(); break;
        case 63: _t->moveLineDown(); break;
        case 64: _t->sortLinesAscending(); break;
        case 65: _t->sortLinesDescending(); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10MainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 66)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 66;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 66)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 66;
    }
    return _id;
}
QT_WARNING_POP
