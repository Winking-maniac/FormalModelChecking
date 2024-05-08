SOURCE=ltl.cpp fsm.cpp test.cpp
TARGET=test

CFLAGS = -I. -Wall -Werror --std=c++17 -g

all: $(TARGET)

$(TARGET): $(SOURCE)
	g++ $(CFLAGS) -o $@ -I. $^

run:
	./$(TARGET)

clean:
	rm -rf $(TARGET)
