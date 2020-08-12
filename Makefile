CC=g++
CFLAGS=-c -Wall -std=c++17
LDFLAGS=
SOURCES= manager_workers_suma_cli.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE= manager_workers_suma_cli

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lcaf_io -lcaf_core

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(EXECUTABLE) *.o

