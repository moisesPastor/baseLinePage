//Compilar: g++ -o ext_rand_trees ext_rand_trees.cpp `pkg-config opencv --cflags --libs` 
//g++ -o ext_rand_trees ext_rand_trees.cpp -I/usr/include/opencv -I/usr/include -L/usr/lib  -lopencv_highgui -lopencv_imgproc -lopencv_core -lopencv_ml -O3
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>

#include <opencv/ml.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#define MODE_TRAIN 0
#define MODE_CLASSIF 1

using namespace cv;
using namespace std;


void usage(char * nomProg){
    cerr << "Usage: " << nomProg << " [options]" << endl;
    cerr << "    options:" << endl;
    cerr << "       -i input filename (REQUIRED, data for training or classifying)" << endl;
    cerr << "       -l imput file list (REQUIRED, data for training or classifying)" << endl;
    cerr << "       -m mode (\"train\" or \"classify\", default \"classify\")" << endl;
    cerr << "       -t tree filename (REQUIRED, in training mode, the file to save the tree, in classification, the tree to use)" << endl;
    cerr << "       -n number of samples required to split a node (used in training, default 2)" << endl;
    cerr << "       -k number of random splits at each node (used in training, default sqrt(number_of_samples))" << endl;
    cerr << "       -s number of trees (used in training, default 100)" << endl;
}

void displayTrainingParameters(string &inFile, string &treeFile, int max_depth, int nMin, int K, int M){
    cout << "Training ERT with data from " << inFile << ", with parameters:" << endl;
    cout << "   nMin = " << nMin << endl;
    cout << "   K = " << K << endl;
    cout << "   M = " << M << endl;
    cout << "Output trained ERT to " << treeFile << endl;
}

void trainERT(string &inFile, string &treeFile, int nMin, int K, int M){
  // Read the training data
  CvMLData dataManager;    
  int ret_val = dataManager.read_csv(inFile.c_str());
  if(ret_val == -1){
    cerr << "Error reading the input data " << endl;
    return;
  }
  dataManager.change_var_type(0, CV_VAR_CATEGORICAL); // The first column has the labels
  dataManager.set_response_idx(0);

     /* max_depth shouldn't be used, nMin limits the tree depth
       
	calc_var_importance should be set to false, but setting it to
       true could be interesting in the future to see if there are
       areas that are systematically non informative. 

       If we set term_criteria to CV_TERMCRIT_ITER, M is used to
       determine when to stop (max numer of trees), if we set it to
       CV_TERMCRIT_EPS, forest_accuracy is used. We can use both (one
       | theother) too.
       */
    
    int max_depth = 1000;
    int max_categories = 2;
    bool calc_var_importance = true;    
    float forest_accuracy = 1e-6; // Not used
    int term_criteria = CV_TERMCRIT_ITER+CV_TERMCRIT_EPS;
    
    CvRTParams* parameters = new CvRTParams(max_depth, nMin, 0.0, false, max_categories,
					    0, calc_var_importance, 
                                            K, M, forest_accuracy, term_criteria);
    
    CvERTrees ert;
    displayTrainingParameters(inFile, treeFile, max_depth, nMin, K, M);

    ert.train(&dataManager,*parameters);
    ert.save(treeFile.c_str());
    
    if (calc_var_importance){
      Mat vars = ert.getVarImportance();
      for (int i = 0; i < vars.rows; i++) {
	cout << vars.row(i) << endl;
      }
    }
    
    delete parameters;
}


void classifyERT( vector<string> & files, string &treeFile, float minProb){
  CvERTrees ert;
  ert.load(treeFile.c_str()); 
  
  for(int i=0;i<files.size();i++){
    CvMLData dataManager;
    int ret_val = dataManager.read_csv(files[i].c_str()); // Read the data
	    
    if(ret_val == -1){
      cerr << "Error reading the input data for file " << files[i] << endl;
    }else{
      Mat* data = new Mat(dataManager.get_values());
	      
      string outname = files[i];
      outname.append(".clas");
      ofstream out;
      out.open(outname.c_str());

      for(int j=0;j<data->rows;j++){  //CvForestTree* get_tree(int i) const;
	//cout << data->row(j)<< endl;
	float clas = ert.predict(data->row(j));
	
	float cont=0;
	int ntrees = ert.get_tree_count();
	for (int t=0; t < ntrees; t++){
	  CvForestTree* tree = ert.get_tree(t);
	  CvDTreeNode* node= tree->predict(data->row(j));
	  if (node->value == clas)
	    cont ++;
    
	}
	// if (clas == 10 && cont/(float)ntrees <=minProb)          
	//   clas = 2;

	out << clas << endl; //"  "<<cont/ntrees<< endl;
	
      }
      out.close();
      delete data;
    }    
  }
}


int main(int argc, char ** argv){
    int option;
    string inFile="";
    string listInFile="";
    string treeFile="";
    int mode=2; // Default
    
    // ERT parameters for training
    int nMin = 2;
    int K = 0; // Default value of sqrt N
    int M = 100;
    float minProb=0.5;

    string m; // Auxiliar
    if(argc<3){
        usage(argv[0]);
        return -1;
    }
    while ((option=getopt(argc,argv,"hi:l:m:t:n:k:s:p:"))!=-1)
        switch (option)  {
            case 'i': // Input data for training, testing or classification
                inFile=optarg;
                break;
	     case 'l':
                listInFile=optarg;
                break;
	     case 'm': // Select mode
                m=optarg;
                if(m=="train")
                    mode = MODE_TRAIN;
                else if(m=="classify")
                    mode = MODE_CLASSIF;
                else{
                    cerr << "Wrong mode. Possible choices are \"train\" or \"classify\"" << endl;
                    return -1;
                }            
                break;
            case 't': // tree file name (for saving in training, or the file that contains the trained tree for test/classification
                treeFile=optarg;
                break;
            case 'n': // Number of samples required to split a node
                nMin = atoi(optarg);
                break;
            case 'k': // Number of random splits at each node
                K = atoi(optarg);
                break;
            case 's': // Number of trees (size of the forest)
                M = atoi(optarg);
                break;
	    case 'p':
                minProb=atof(optarg);
                break;

            case 'h':
            default:
                usage(argv[0]);
                return 1;
        }
    if(inFile=="" && listInFile==""){
        cerr << "Error: no input data filename" << endl;
        return -1;
    }
    if(treeFile==""){
        cerr << "Error: no tree filename" << endl;
        return -1;
    }
    switch(mode){
        case MODE_TRAIN:
            trainERT(inFile, treeFile, nMin, K, M);
            break;
        case MODE_CLASSIF:{
	  vector<string> files;
	  
	  if(inFile.size()!=0)
	    files.push_back(inFile);
	  else{
	    ifstream in;
	    in.open(listInFile.c_str());
	    while(in.good()){
	      string tmp;
	      in >> tmp;
	      if(tmp!="")
                files.push_back(tmp);
	    }
	  }
	  classifyERT(files, treeFile,minProb);
	  break;
	}
       default:

	 cerr << argv[0]<< " --> Error: mode must be specified"<< endl;
    } 
    return 0;

      
}
