CC=g++
CFLAGS=-c -Wall -std=c++17
LDFLAGS=
SOURCES= manager_workers_suma.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE= manager_workers_suma

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lcaf_io -lcaf_core

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) *.o

