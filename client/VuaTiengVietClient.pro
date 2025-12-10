QT       += core gui network widgets

TARGET = VuaTiengVietClient
TEMPLATE = app

# Cấu hình đường dẫn để include file dễ dàng hơn
# Ví dụ: chỉ cần #include "GameClient.h" thay vì #include "../../network/GameClient.h"
INCLUDEPATH += $$PWD/src \
               $$PWD/src/core \
               $$PWD/src/network \
               $$PWD/src/models \
               $$PWD/src/ui/auth \
               $$PWD/src/ui/lobby \
               $$PWD/src/ui/game \
               $$PWD/src/ui/components \
               $$PWD/src/utils

# Định nghĩa các file Header
HEADERS += \
    src/core/MainWindow.h \
    src/network/GameClient.h \
    src/network/Protocol.h \
    src/ui/auth/LoginWidget.h \
    src/ui/auth/RegisterWidget.h

# Định nghĩa các file Source
SOURCES += \
    src/main.cpp \
    src/core/MainWindow.cpp \
    src/network/GameClient.cpp \
    src/ui/auth/LoginWidget.cpp \
    src/ui/auth/RegisterWidget.cpp

# Tài nguyên (Nếu có file resources.qrc)
# RESOURCES += resources.qrc