QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
INCLUDEPATH += \
    $$PWD/ExternalLibs/glm \
    $$PWD/ExternalLibs/glew/include \
    $$PWD/ExternalLibs/glfw/include \
    $$PWD/ExternalLibs/exprtk \
    $$PWD/ExternalLibs/learnply \
    $$PWD/ExternalLibs/eigen

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ExternalLibs/learnply/learnply.cpp \
    ExternalLibs/learnply/ply.cpp \
    main.cpp \
    mainwindow.cpp \
    mywidget.cpp \
    object.cpp \
    objectcomplex.cpp \
    objectmodel.cpp \
    objectpoly.cpp \
    objectsilhouette.cpp \
    objectsurface.cpp \
    surfaceconfig.cpp

HEADERS += \
    ExternalLibs/learnply/icVector.H \
    ExternalLibs/learnply/learnply.h \
    ExternalLibs/learnply/ply.h \
    ExternalLibs/learnply/vectorField.h \
    mainwindow.h \
    mywidget.h \
    object.h \
    objectcomplex.h \
    objectmodel.h \
    objectpoly.h \
    objectsilhouette.h \
    objectsurface.h \
    surfaceconfig.h

FORMS += \
    mainwindow.ui \
    surfaceconfig.ui

LIBS += \
    $$PWD/ExternalLibs/glew/lib/Release/x64/glew32.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    shaders/Box.frag \
    shaders/Box.vert \
    shaders/Light.frag \
    shaders/Light.vert \
    shaders/Mesh.frag \
    shaders/Mesh.vert

QMAKE_CXXFLAGS += /bigobj \
                /FS
