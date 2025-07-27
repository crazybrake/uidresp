CXX := clang++
CXXFLAGS := -std=c++17 -stdlib=libc++ -Wall -Wextra -O2
CXXFLAGS += -I/opt/homebrew/include
LDFLAGS := -L/opt/homebrew/lib -lgtest -lgtest_main
TARGET := uidresp 
SRC := main.cpp
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@

%.o: %.cpp uidresp.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

TEST_SRC := tests/test_uidresp.cpp
TEST_BIN := test_uidresp
TEST_OBJ := $(TEST_SRC:.cpp=.o)


$(TEST_BIN): $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(TEST_OBJ) $(LDFLAGS) -lpthread -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_OBJ) $(TEST_BIN)


