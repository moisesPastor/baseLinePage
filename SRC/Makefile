
OPEN_LIBS=-lopencv_imgproc -lopencv_core  -lopencv_highgui
OTHER_LIBS=-lpugixml

all: baseLinePage GTTool 
baseLinePage: 
	mkdir ../BIN || echo "../BIN directory already exists"
	make -C LIB
	make -C clusterLinies;
	make -C imageLocalExtrema 
	make -C Extremely_Randomized_Trees
	make -C labelPointsFromBaselines
	make -C lineExtractor 
	g++ -o pointsToClassifierData pointsToClassifierData.cc ${OPEN_LIBS} -O3
	g++ -o drawBaselines drawBaselines.cc  LIB/libPoints.o LIB/libXmlPAGE.o -ILIB ${OPEN_LIBS} ${OTHER_LIBS} -O3
	cp clusterLinies/mdbscan imageLocalExtrema/imageLocalExtrema Extremely_Randomized_Trees/ext_rand_trees labelPointsFromBaselines/labelPointsFromBaselines pointsToClassifierData drawBaselines lineExtractor/lineExtractor TranskribusBaseLineEvaluationScheme-0.1.3-jar-with-dependencies.jar ../BIN

GTTool:
	qmake-qt4 -o GT_Tool_PAGE_Points/Makefile GT_Tool_PAGE_Points/GT_Tool_PAGE_Points.pro
	make  -C GT_Tool_PAGE_Points/ 
	cp GT_Tool_PAGE_Points/GT_Tool_PAGE_Points ../BIN

clean:
	rm pointsToClassifierData
	rm drawBaselines
	make -C clusterLinies clean;
	make -C imageLocalExtrema clean;
	make -C Extremely_Randomized_Trees clean
	make -C labelPointsFromBaselines clean
	make -C lineExtractor clean
	make -C GT_Tool_PAGE_Points clean
	make -C GT_Tool_PAGE_Points distclean
