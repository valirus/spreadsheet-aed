CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2
SFML     = -lsfml-graphics -lsfml-window -lsfml-system
SRC      = src/sparse_matrix.cpp src/operations.cpp src/gui.cpp src/main.cpp
TARGET   = spreadsheet

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(SFML)

clean:
	rm -f $(TARGET)