CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
LDFLAGS := -lsqlite3 -lssl -lcrypto -lpthread

SRC := main.cpp \
       core/Server.cpp core/ClientHandler.cpp core/Dispatcher.cpp core/MassageParser.cpp \
       database/DB.cpp database/UserDAO.cpp database/QuestionDAO.cpp database/FriendDAO.cpp database/MatchDAO.cpp \
       models/Friendship.h models/Match.h models/Question.h models/Room.h models/User.h \
       protocol/Massages.cpp \
       services/UserService.cpp services/FriendService.cpp services/RoomService.cpp services/MatchmakingService.cpp services/RematchService.cpp services/MatchService.cpp services/GameService.cpp \
       utils/Crypto.cpp

# Filter only .cpp files
SRC := $(filter %.cpp,$(SRC))

OBJ := $(SRC:.cpp=.o)
TARGET := server_app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
