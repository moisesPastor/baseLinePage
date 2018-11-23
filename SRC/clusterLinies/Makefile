all: mdbscan
OPEN_LIBS=`pkg-config --cflags --libs opencv` -lpugixml
INCLUDES=-I. -I../LIB


mdbscan: mdbscan.cc  slopeClass.cc 
	make -C ../LIB
	g++ -o mdbscan mdbscan.cc  slopeClass.cc ../LIB/libPoints.o ../LIB/libXmlPAGE.o ${INCLUDES} ${OPEN_LIBS} -O3
clean:
	rm mdbscan ../LIB/libPoints.o ../LIB/libXmlPAGE.o


