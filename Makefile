.PHONY: clean
CXX		  := clang++
CXX_FLAGS := -Wall -Wextra -std=c++11 -framework OpenGL -g

BIN		:= bin
SRC		:= .
INCLUDE	:= imgui imgui-sfml-2.1
LIB		:= /usr/local/lib

LIBRARIES	:= -lfairygui-Mac -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system
EXECUTABLE	:= Hanafuda


all: $(BIN)/$(EXECUTABLE)

run: clean all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp imgui/*.cpp
	$(CXX) $(CXX_FLAGS) $(foreach word,$(INCLUDE),-I$(word)) $(foreach word,$(LIB),-L$(word)) $^ -o $@ $(LIBRARIES)

clean:
	-rm $(BIN)/*