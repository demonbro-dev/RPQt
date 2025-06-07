QT       += core gui network httpserver

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += include

RC_ICONS = $$PWD/data/RandPickerLogo.ico
QMAKE_TARGET_COMPANY = "demonbro"
QMAKE_TARGET_PRODUCT = "RandPicker Qt"
QMAKE_TARGET_DESCRIPTION = "RandPicker Qt"
VERSION= "1.0.0.0"
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/aboutdialog.cpp \
    src/jsonhandler.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/namemanager.cpp \
    src/passphrasedialog.cpp \
    src/pickerlogic.cpp \
    src/pickhistorydialog.cpp \
    src/rpweb.cpp \
    src/scheduledpickdialog.cpp \
    src/sidebutton.cpp

HEADERS += \
    include/aboutdialog.h \
    include/jsonhandler.h \
    include/mainwindow.h \
    include/namemanager.h \
    include/passphrasedialog.h \
    include/pickerlogic.h \
    include/pickhistorydialog.h \
    include/rpweb.h \
    include/scheduledpickdialog.h \
    include/sidebutton.h

FORMS += \
    ui/aboutdialog.ui \
    ui/mainwindow.ui \
    ui/namemanager.ui \
    ui/passphrasedialog.ui \
    ui/pickhistorydialog.ui \
    ui/rpweb.ui \
    ui/scheduledpickdialog.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TRANSLATIONS += translations/RPQt_zh_CN.ts \
                translations/RPQt_en_US.ts
