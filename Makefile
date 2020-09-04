PROJECT     := javsh

RELEASE     := -O3
DEBUG       := -ggdb3 -Og
STATIC      := -Bstatic -static-libgcc -static-libstdc++
SHARED      := -static-libstdc++

BUILD       := $(DEBUG)
#BUILD      := $(RELEASE)

LINKTYPE    := $(STATIC)
#LINKTYPE   := $(SHARED)

CXX_FLAGS   := -std=c++17 $(BUILD) $(LINKTYPE)
CXX         := g++

BIN         := bin
SRC         := src
INC         := include
LIB         := lib
OBJ         := obj
RES         := res

INC_FLAG    := -I$(INC)
LIB_FLAG    := -L$(LIB)

ifeq ($(OS),Windows_NT)
	EXECUTABLE  := $(PROJECT).exe
	PLATFORM    := mingw
	LIBRARIES   := 
	CLEAN_COMMAND := del $(BIN)\*.exe $(OBJ)$(PLATFORM)\*.o
else
	EXECUTABLE  := $(PROJECT)
	PLATFORM    := linux-x86_64
	LIBRARIES   := 
	CLEAN_COMMAND := -rm $(BIN)/* $(OBJ)/$(PLATFORM)/*.o
endif

SOURCES     := $(wildcard $(SRC)/*.cpp)
OBJECTS     := $(patsubst $(SRC)/%,$(OBJ)/$(PLATFORM)/%,$(SOURCES:.cpp=.o))

.PHONY: clean all

all: $(BIN)/$(EXECUTABLE)

clean:
	$(CLEAN_COMMAND)

# Compile only
$(OBJ)/$(PLATFORM)/%.o : $(SRC)/%.cpp $(DEPENDENCIES)
	$(CXX) $(CXX_FLAGS) $(INC_FLAG) -c -o $@ $<

# Link the object files and libraries
$(BIN)/$(EXECUTABLE) : $(OBJECTS)
	$(CXX) $(CXX_FLAGS) -o $(BIN)/$(EXECUTABLE) $^ $(LIBRARIES) $(LIB_FLAG)

