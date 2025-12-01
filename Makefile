CXX = g++ -fopenmp
CXXFLAGS = -I/usr/include/eigen3 -std=c++17 -Wall -Wextra
LDFLAGS = -lSDL2
TARGET = rasterizer
SRC = rasterizer.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run