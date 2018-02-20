all: lineExtractor
OPEN_LIBS=-lopencv_imgproc -lopencv_core -lopencv_highgui -lpugixml
lineExtractor: lineExtractor.cc  slopeClass.cc Graf.cc
	g++ -o lineExtractor lineExtractor.cc Graf.cc slopeClass.cc -I. -O3 ${OPEN_LIBS} 

clean:
	rm lineExtractor *.o


