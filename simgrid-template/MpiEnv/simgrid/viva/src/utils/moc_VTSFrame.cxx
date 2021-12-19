/****************************************************************************
** Meta object code from reading C++ file 'VTSFrame.h'
**
** Created: Tue Nov 25 12:11:06 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "VTSFrame.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VTSFrame.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_VTSFrame[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   10,    9,    9, 0x0a,
      44,   38,    9,    9, 0x0a,
      67,   10,    9,    9, 0x0a,
      88,   38,    9,    9, 0x0a,
     110,   10,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_VTSFrame[] = {
    "VTSFrame\0\0value\0startSliderMoved(int)\0"
    "state\0startCheckChanged(int)\0"
    "sizeSliderMoved(int)\0sizeCheckChanged(int)\0"
    "startSizeSliderReleased(int)\0"
};

void VTSFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VTSFrame *_t = static_cast<VTSFrame *>(_o);
        switch (_id) {
        case 0: _t->startSliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->startCheckChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->sizeSliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->sizeCheckChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->startSizeSliderReleased((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VTSFrame::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VTSFrame::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_VTSFrame,
      qt_meta_data_VTSFrame, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VTSFrame::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VTSFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VTSFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VTSFrame))
        return static_cast<void*>(const_cast< VTSFrame*>(this));
    if (!strcmp(_clname, "PajeComponent"))
        return static_cast< PajeComponent*>(const_cast< VTSFrame*>(this));
    return QFrame::qt_metacast(_clname);
}

int VTSFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
