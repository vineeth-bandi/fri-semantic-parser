CXX = g++

SRCS = Ontology.cpp ParseNode.cpp
HEADERS = Ontology.h ParseNode.h
FILES = $(SRCS)
FILE_NAME = ontology

ontology: $(SRCS)
	$(CXX) -o $(FILE_NAME) $(FILES)

clean:
	rm -f $(FILE_NAME) *.o *~