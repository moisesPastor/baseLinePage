/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x0a,
      25,   11,   11,   11, 0x0a,
      37,   11,   11,   11, 0x0a,
      48,   11,   11,   11, 0x0a,
      60,   11,   11,   11, 0x0a,
      91,   75,   11,   11, 0x0a,
     121,  112,   11,   11, 0x2a,
     137,   11,   11,   11, 0x2a,
     148,  112,   11,   11, 0x0a,
     170,   11,   11,   11, 0x2a,
     187,   11,   11,   11, 0x0a,
     199,   11,   11,   11, 0x0a,
     215,   11,   11,   11, 0x0a,
     226,   11,   11,   11, 0x0a,
     245,  239,   11,   11, 0x0a,
     278,  274,   11,   11, 0x0a,
     305,  302,   11,   11, 0x0a,
     332,  323,   11,   11, 0x0a,
     372,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0modeLayout()\0modeLines()\0"
    "showHelp()\0showAbout()\0showSettings()\0"
    "filename,single\0loadFile(char*,bool)\0"
    "filename\0loadFile(char*)\0loadFile()\0"
    "loadPointsFile(char*)\0loadPointsFile()\0"
    "nextImage()\0previousImage()\0saveData()\0"
    "saveDataAs()\0label\0updateLabelCombobox(QString)\0"
    "x,y\0updatePosition(int,int)\0id\0"
    "updateID(QString)\0settings\0"
    "updateSettings(QMap<QString,QVariant>&)\0"
    "loadSingleFile()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->modeLayout(); break;
        case 1: _t->modeLines(); break;
        case 2: _t->showHelp(); break;
        case 3: _t->showAbout(); break;
        case 4: _t->showSettings(); break;
        case 5: _t->loadFile((*reinterpret_cast< char*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 6: _t->loadFile((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 7: _t->loadFile(); break;
        case 8: _t->loadPointsFile((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 9: _t->loadPointsFile(); break;
        case 10: _t->nextImage(); break;
        case 11: _t->previousImage(); break;
        case 12: _t->saveData(); break;
        case 13: _t->saveDataAs(); break;
        case 14: _t->updateLabelCombobox((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 15: _t->updatePosition((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 16: _t->updateID((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 17: _t->updateSettings((*reinterpret_cast< QMap<QString,QVariant>(*)>(_a[1]))); break;
        case 18: _t->loadSingleFile(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
