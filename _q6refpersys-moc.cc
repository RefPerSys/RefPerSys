/****************************************************************************
** Meta object code from reading C++ file 'q6refpersys.cc'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'q6refpersys.cc' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN15MyqrApplicationE_t {};
} // unnamed namespace

template <> constexpr inline auto MyqrApplication::qt_create_metaobjectdata<qt_meta_tag_ZN15MyqrApplicationE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MyqrApplication",
        "compilation_process_started",
        "",
        "compilation_process_finished",
        "exitCode",
        "QProcess::ExitStatus",
        "exitStatus",
        "compilation_process_changed_state",
        "QProcess::ProcessState",
        "newstate",
        "compilation_process_errored",
        "QProcess::ProcessError",
        "error",
        "compilation_process_readable"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'compilation_process_started'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'compilation_process_finished'
        QtMocHelpers::SlotData<void(int, QProcess::ExitStatus)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Slot 'compilation_process_finished'
        QtMocHelpers::SlotData<void(int)>(3, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Slot 'compilation_process_changed_state'
        QtMocHelpers::SlotData<void(QProcess::ProcessState)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Slot 'compilation_process_errored'
        QtMocHelpers::SlotData<void(QProcess::ProcessError)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'compilation_process_readable'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MyqrApplication, qt_meta_tag_ZN15MyqrApplicationE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MyqrApplication::staticMetaObject = { {
    QMetaObject::SuperData::link<QApplication::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15MyqrApplicationE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15MyqrApplicationE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15MyqrApplicationE_t>.metaTypes,
    nullptr
} };

void MyqrApplication::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyqrApplication *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->compilation_process_started(); break;
        case 1: _t->compilation_process_finished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        case 2: _t->compilation_process_finished((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->compilation_process_changed_state((*reinterpret_cast<std::add_pointer_t<QProcess::ProcessState>>(_a[1]))); break;
        case 4: _t->compilation_process_errored((*reinterpret_cast<std::add_pointer_t<QProcess::ProcessError>>(_a[1]))); break;
        case 5: _t->compilation_process_readable(); break;
        default: ;
        }
    }
}

const QMetaObject *MyqrApplication::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyqrApplication::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15MyqrApplicationE_t>.strings))
        return static_cast<void*>(this);
    return QApplication::qt_metacast(_clname);
}

int MyqrApplication::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
namespace {
struct qt_meta_tag_ZN11MyqrProcessE_t {};
} // unnamed namespace

template <> constexpr inline auto MyqrProcess::qt_create_metaobjectdata<qt_meta_tag_ZN11MyqrProcessE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MyqrProcess",
        "get",
        "",
        "std::string",
        "put",
        "remove",
        "repeat_until",
        "std::function<MyqrProcess_until_sigt>",
        "fun",
        "data",
        "const MyqrProcess_until_sigt*",
        "pfun"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'get'
        QtMocHelpers::SlotData<QObject *(const std::string &) const>(1, 2, QMC::AccessPublic, QMetaType::QObjectStar, {{
            { 0x80000000 | 3, 2 },
        }}),
        // Slot 'put'
        QtMocHelpers::SlotData<void(const std::string &, QObject *)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 2 }, { QMetaType::QObjectStar, 2 },
        }}),
        // Slot 'remove'
        QtMocHelpers::SlotData<void(const std::string)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 2 },
        }}),
        // Slot 'repeat_until'
        QtMocHelpers::SlotData<bool(const std::function<MyqrProcess_until_sigt> &, void *) const>(6, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 7, 8 }, { QMetaType::VoidStar, 9 },
        }}),
        // Slot 'repeat_until'
        QtMocHelpers::SlotData<bool(const std::function<MyqrProcess_until_sigt> &) const>(6, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Bool, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'repeat_until'
        QtMocHelpers::SlotData<bool(const MyqrProcess_until_sigt *, void *) const>(6, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 10, 11 }, { QMetaType::VoidStar, 9 },
        }}),
        // Slot 'repeat_until'
        QtMocHelpers::SlotData<bool(const MyqrProcess_until_sigt *) const>(6, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Bool, {{
            { 0x80000000 | 10, 11 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MyqrProcess, qt_meta_tag_ZN11MyqrProcessE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MyqrProcess::staticMetaObject = { {
    QMetaObject::SuperData::link<QProcess::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyqrProcessE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyqrProcessE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11MyqrProcessE_t>.metaTypes,
    nullptr
} };

void MyqrProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyqrProcess *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: { QObject* _r = _t->get((*reinterpret_cast<std::add_pointer_t<std::string>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QObject**>(_a[0]) = std::move(_r); }  break;
        case 1: _t->put((*reinterpret_cast<std::add_pointer_t<std::string>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QObject*>>(_a[2]))); break;
        case 2: _t->remove((*reinterpret_cast<std::add_pointer_t<std::string>>(_a[1]))); break;
        case 3: { bool _r = _t->repeat_until((*reinterpret_cast<std::add_pointer_t<std::function<MyqrProcess_until_sigt>>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<void*>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 4: { bool _r = _t->repeat_until((*reinterpret_cast<std::add_pointer_t<std::function<MyqrProcess_until_sigt>>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { bool _r = _t->repeat_until((*reinterpret_cast<std::add_pointer_t<const MyqrProcess_until_sigt*>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<void*>>(_a[2])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 6: { bool _r = _t->repeat_until((*reinterpret_cast<std::add_pointer_t<const MyqrProcess_until_sigt*>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *MyqrProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyqrProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11MyqrProcessE_t>.strings))
        return static_cast<void*>(this);
    return QProcess::qt_metacast(_clname);
}

int MyqrProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QProcess::qt_metacall(_c, _id, _a);
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
    return _id;
}
namespace {
struct qt_meta_tag_ZN14MyqrMainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MyqrMainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN14MyqrMainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MyqrMainWindow",
        "about",
        "",
        "aboutQt",
        "toggle_debug"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'about'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'aboutQt'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggle_debug'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MyqrMainWindow, qt_meta_tag_ZN14MyqrMainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MyqrMainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14MyqrMainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14MyqrMainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14MyqrMainWindowE_t>.metaTypes,
    nullptr
} };

void MyqrMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyqrMainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->about(); break;
        case 1: _t->aboutQt(); break;
        case 2: _t->toggle_debug(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *MyqrMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyqrMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14MyqrMainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MyqrMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
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
namespace {
struct qt_meta_tag_ZN17MyqrDisplayWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto MyqrDisplayWindow::qt_create_metaobjectdata<qt_meta_tag_ZN17MyqrDisplayWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MyqrDisplayWindow",
        "about",
        ""
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'about'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MyqrDisplayWindow, qt_meta_tag_ZN17MyqrDisplayWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MyqrDisplayWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MyqrDisplayWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MyqrDisplayWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17MyqrDisplayWindowE_t>.metaTypes,
    nullptr
} };

void MyqrDisplayWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MyqrDisplayWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->about(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *MyqrDisplayWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyqrDisplayWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17MyqrDisplayWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MyqrDisplayWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
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
