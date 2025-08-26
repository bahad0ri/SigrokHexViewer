/****************************************************************************
** Meta object code from reading C++ file 'sigrokworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../sigrokworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sigrokworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SigrokWorker_t {
    QByteArrayData data[13];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SigrokWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SigrokWorker_t qt_meta_stringdata_SigrokWorker = {
    {
QT_MOC_LITERAL(0, 0, 12), // "SigrokWorker"
QT_MOC_LITERAL(1, 13, 12), // "rawLineReady"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 4), // "line"
QT_MOC_LITERAL(4, 32, 13), // "canFrameReady"
QT_MOC_LITERAL(5, 46, 8), // "CanFrame"
QT_MOC_LITERAL(6, 55, 1), // "f"
QT_MOC_LITERAL(7, 57, 6), // "logMsg"
QT_MOC_LITERAL(8, 64, 3), // "msg"
QT_MOC_LITERAL(9, 68, 8), // "finished"
QT_MOC_LITERAL(10, 77, 4), // "code"
QT_MOC_LITERAL(11, 82, 5), // "start"
QT_MOC_LITERAL(12, 88, 4) // "stop"

    },
    "SigrokWorker\0rawLineReady\0\0line\0"
    "canFrameReady\0CanFrame\0f\0logMsg\0msg\0"
    "finished\0code\0start\0stop"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SigrokWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    1,   47,    2, 0x06 /* Public */,
       7,    1,   50,    2, 0x06 /* Public */,
       9,    1,   53,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   56,    2, 0x0a /* Public */,
      12,    0,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SigrokWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SigrokWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->rawLineReady((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->canFrameReady((*reinterpret_cast< const CanFrame(*)>(_a[1]))); break;
        case 2: _t->logMsg((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->finished((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->start(); break;
        case 5: _t->stop(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SigrokWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SigrokWorker::rawLineReady)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SigrokWorker::*)(const CanFrame & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SigrokWorker::canFrameReady)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SigrokWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SigrokWorker::logMsg)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SigrokWorker::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SigrokWorker::finished)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SigrokWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_SigrokWorker.data,
    qt_meta_data_SigrokWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SigrokWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SigrokWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SigrokWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SigrokWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SigrokWorker::rawLineReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SigrokWorker::canFrameReady(const CanFrame & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SigrokWorker::logMsg(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SigrokWorker::finished(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
