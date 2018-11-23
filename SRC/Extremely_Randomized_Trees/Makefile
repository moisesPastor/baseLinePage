OPENCV_LIBS=`pkg-config --cflags --libs opencv`
PROGS=ext_rand_trees
OPTIM=-O3
all:${PROGS}

ext_rand_trees: ext_rand_trees.cc
	g++ -o ext_rand_trees ext_rand_trees.cc ${OPENCV_LIBS} ${OPTIM}

clean:
	rm ext_rand_trees
