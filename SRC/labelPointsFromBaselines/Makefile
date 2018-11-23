all: labelPointsFromBaselines
OPEN_LIBS=`pkg-config --cflags --libs opencv` -lpugixml
labelPointsFromBaselines: labelPointsFromBaselines.cc
	g++ -o  labelPointsFromBaselines labelPointsFromBaselines.cc ${OPEN_LIBS} -O3

clean:
	rm labelPointsFromBaselines
