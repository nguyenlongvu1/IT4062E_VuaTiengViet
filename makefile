# ==========================================
# COMPILER CONFIG
# ==========================================
CXX = g++
CXXFLAGS = -std=c++17 -pthread -Wall -O2

# ==========================================
# SERVER
# ==========================================
SERVER_DIR      = server
SERVER_TARGET   = server_app

SERVER_SUBDIRS  = $(SERVER_DIR) \
                  $(SERVER_DIR)/core \
                  $(SERVER_DIR)/protocol \
                  $(SERVER_DIR)/services \
                  $(SERVER_DIR)/models \
                  $(SERVER_DIR)/database \
                  $(SERVER_DIR)/utils

SERVER_SRC      = $(foreach dir,$(SERVER_SUBDIRS),$(wildcard $(dir)/*.cpp))
SERVER_OBJ      = $(SERVER_SRC:.cpp=.o)

# ==========================================
# BUILD ALL
# ==========================================
all: $(SERVER_TARGET)

server: $(SERVER_TARGET)

# ==========================================
# SERVER BUILD RULE
# ==========================================
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJ) -lsqlite3 -lcrypto

# ==========================================
# GENERIC RULE FOR .cpp -> .o
# ==========================================
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# RUN HELPERS
# ==========================================
run_server: server
	./$(SERVER_TARGET)

# ==========================================
# CLEAN
# ==========================================
clean:
	rm -f $(SERVER_OBJ) $(SERVER_TARGET)

rebuild: clean all