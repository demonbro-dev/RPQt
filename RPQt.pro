QT       += core gui network websockets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += include
INCLUDEPATH += third_party
QMAKE_CXXFLAGS += -mrdrnd

RC_ICONS = $$PWD/data/RandPickerLogo.ico
QMAKE_TARGET_COMPANY = "demonbro"
QMAKE_TARGET_PRODUCT = "RandPicker"
QMAKE_TARGET_DESCRIPTION = "RandPicker"
VERSION= "1.0.0.0"
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/aboutdialog.cpp \
    src/fbshandler.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/namemanager.cpp \
    src/passphrasedialog.cpp \
    src/pickerlogic.cpp \
    src/pickhistorydialog.cpp \
    src/randmirage.cpp \
    src/rpweb.cpp \
    src/scheduledpickdialog.cpp \
    src/settingshandler.cpp \
    src/sidebutton.cpp \
    src/updater.cpp \
    src/visualeditor.cpp

HEADERS += \
    include/aboutdialog.h \
    include/fbshandler.h \
    include/mainwindow.h \
    include/namemanager.h \
    include/passphrasedialog.h \
    include/pickerlogic.h \
    include/pickhistorydialog.h \
    include/randmirage.h \
    include/rpweb.h \
    include/scheduledpickdialog.h \
    include/settingshandler.h \
    include/sidebutton.h \
    include/updater.h \
    include/visualeditor.h

FORMS += \
    ui/aboutdialog.ui \
    ui/mainwindow.ui \
    ui/namemanager.ui \
    ui/passphrasedialog.ui \
    ui/pickhistorydialog.ui \
    ui/rpweb.ui \
    ui/scheduledpickdialog.ui \
    ui/updater.ui

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TRANSLATIONS += translations/RPQt_zh_CN.ts \
                translations/RPQt_en_US.ts
win32 {
    LIBS += -luser32 -lbcrypt
}
win32:release {
    QMAKE_EXTRA_TARGETS += write_git_commit
    PRE_TARGETDEPS += write_git_commit
    write_git_commit.commands = @echo Built on commit: [$$system(git rev-parse --short=8 HEAD)]\(https://github.com/demonbro-dev/RPQt/commit/$$system(git rev-parse HEAD)\) > $$PWD/data/commit.md || echo. > $$PWD/data/commit.md
}
