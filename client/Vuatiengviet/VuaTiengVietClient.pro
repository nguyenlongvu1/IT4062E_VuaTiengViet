QT       += core gui network widgets multimedia

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
               $$PWD/src/utils\
               $$PWD/src/ui/home

# Định nghĩa các file Header
HEADERS += \
    src/core/MainWindow.h \
    src/network/GameClient.h \
    src/network/Protocol.h \
    src/ui/auth/LoginWidget.h \
    src/ui/auth/RegisterWidget.h \
    src/ui/home/HomeWidget.h \
    src/ui/home/LeaderboardWidget.h \
    src/ui/home/SocialWidget.h \
    src/ui/home/SettingsDialog.h \
    src/ui/home/ProfileDialog.h \
    src/ui/room/MatchmakingWidget.h \
    src/ui/room/FriendRoomWidget.h \
    src/ui/home/NotificationDialog.h \
    src/ui/home/ChangePasswordDialog.h \
    src/utils/AudioManager.h \
    src/ui/game/GameWidget.h \
    src/ui/game/ResultWidget.h \
    src/utils/GameButton.h
    

# Định nghĩa các file Source
SOURCES += \
    src/main.cpp \
    src/core/MainWindow.cpp \
    src/network/GameClient.cpp \
    src/ui/auth/LoginWidget.cpp \
    src/ui/auth/RegisterWidget.cpp \
    src/ui/home/HomeWidget.cpp \
    src/ui/home/LeaderboardWidget.cpp \
    src/ui/home/SocialWidget.cpp \
    src/ui/home/SettingsDialog.cpp \
    src/ui/home/ProfileDialog.cpp \
    src/ui/room/MatchmakingWidget.cpp \
    src/ui/room/FriendRoomWidget.cpp \
    src/ui/home/NotificationDialog.cpp \
    src/utils/AudioManager.cpp \
    src/ui/game/GameWidget.cpp \
    src/ui/game/ResultWidget.cpp \
    src/ui/home/ChangePasswordDialog.cpp \
    src/utils/GameButton.cpp

# Tài nguyên (Nếu có file resources.qrc)
# RESOURCES += resources.qrc

RESOURCES += \
    resources.qrc
 LIBS += -lSDL2 -lSDL2_mixer
