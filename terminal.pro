QT += widgets serialport
QT += charts
#QT += pdf

requires(qtConfig(combobox))

TRANSLATIONS += QtLanguage_ru.ts

VERSION = 0.0.0.25

#define ORGANIZATION_NAME "AO NIIMA PROGRESS"
#define ORGANIZATION_DOMAIN "www.mri-progress.ru"
#define APPLICATION_NAME "PROGRESS TERMINAL"

QMAKE_TARGET_COMPANY = AO NIIMA PROGRESS
QMAKE_TARGET_PRODUCT = PROGRESS TERMINAL
QMAKE_TARGET_DESCRIPTION = Modem terminal
QMAKE_TARGET_COPYRIGHT = s.nuzhnov i-progress.tech

TARGET = terminal
TEMPLATE = app

SOURCES += \
    bandsdialog.cpp \
    bluetoothdialog.cpp \
    dataconnectdialog.cpp \
    editcommanddialog.cpp \
    editpdpdialog.cpp \
    gnssdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    mobiledialog.cpp \
    networkdialog.cpp \
    notebookdialog.cpp \
    settingsdialog.cpp \
    console.cpp

HEADERS += \
    bandsdialog.h \
    bluetoothdialog.h \
    dataconnectdialog.h \
    editcommanddialog.h \
    editpdpdialog.h \
    globals.h \
    gnssdialog.h \
    mainwindow.h \
    mobiledialog.h \
    networkdialog.h \
    notebookdialog.h \
    settingsdialog.h \
    console.h

FORMS += \
    bandsdialog.ui \
    bluetoothdialog.ui \
    dataconnectdialog.ui \
    editcommanddialog.ui \
    editpdpdialog.ui \
    gnssdialog.ui \
    mainwindow.ui \
    mobiledialog.ui \
    networkdialog.ui \
    notebookdialog.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

RC_ICONS = images/Log.ico

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target

DISTFILES +=
