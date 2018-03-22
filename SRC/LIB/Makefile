OPEN_LIBS=-lopencv_imgproc -lopencv_core -lopencv_highgui -lpugixml
INCLUDES=-I. -I../LIB

all: libPoints.o libXmlPAGE.o

libPoints.o: libPoints.cc  libPoints.h
	g++ -c libPoints.cc  ${INCLUDES} ${OPEN_LIBS} -O3

libXmlPAGE.o: libXmlPAGE.cc libXmlPAGE.h
	g++ -c libXmlPAGE.cc ${INCLUDES} ${OPEN_LIBS} -O3
clean:
	rm libPoints.o libXmlPAGE.o


