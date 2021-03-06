/*
 *   Copyright 2017, Moisés Pastor i Gadea
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 *  Created on: 20/10/2017
 *      Author: Moisés Pastor i Gadea
 */
#include <unistd.h>

#include <map>
#include <sstream>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>

#include <vector>
#include <algorithm>

#include "pugixml.hpp"
#include "slopeClass.h"
#include "libXmlPAGE.h"
#include "libPoints.h"

using namespace cv;
using namespace std;

const int NOISE=-1;
const int PENALITZACIO_Y=12;

const int numColors=28;
Scalar colors[numColors]={Scalar(255, 23, 0), Scalar(42, 0, 255), Scalar(255, 96, 0), Scalar(155, 0, 255), Scalar(17, 0, 255), Scalar(255, 0, 234), Scalar(0, 41, 255), Scalar(0, 255, 41), Scalar(143, 0, 255),Scalar(30, 0, 255), Scalar(209, 0, 255),  Scalar(255, 0, 0), Scalar(0, 48, 255),  Scalar(255, 0, 108), Scalar(255, 0, 228), Scalar(114, 0, 255), Scalar(255, 156, 0), Scalar(255, 0, 252), Scalar(255, 0, 89), Scalar(0, 0,255), Scalar(222, 0, 255), Scalar(255, 0, 11), Scalar(209, 255, 0), Scalar(0, 83, 255), Scalar(255, 0, 0), Scalar(0, 131, 255), Scalar(252, 0, 255), Scalar(255, 0, 143)};

//---------------------------------------------------------------------
struct  k{
  bool operator() (cv::Point pt1, cv::Point pt2) { 
    if(pt1.x < pt2.x) return true; 
    else if (pt1.x > pt2.x) return false; 
    else return pt1.y < pt2.y ;}
} sort_points_func;

struct k1{
  bool operator() (std::vector< cv::Point > v1, std::vector < cv::Point > v2){
    double sum_y_v1=0;
    for (unsigned int i = 0; i < v1.size(); ++i){
      sum_y_v1+=v1[i].y;
    }

    double sum_y_v2=0;
    for (unsigned int i = 0; i < v2.size(); ++i){
      sum_y_v2+=v2[i].y;
    }

    double mean_y_v1=sum_y_v1/v1.size();
    double mean_y_v2=sum_y_v2/v2.size();
    if (mean_y_v1 <= mean_y_v2) return true;
    else return false;
  }
} sort_lines_func;

struct k2{
  bool operator() (std::vector< cv::Point > v1, std::vector < cv::Point > v2){
    if (v1[v1.size()-1].x < v2[v2.size()-1].x)
      return true;
    else
      return false;
  }
}sort_linesX_func;


//----------------------------------------------------------
class DbScan {
  const static int NOT_VISITED=-99;

public:
  //std::map<int, int> labels;
  //int * labels;
  vector<int> labels;
  vector<Point>& data;
  int numClasses;
  double eps;
  unsigned int min_npts;
  double* dp;
  int verbosity;
  

#define DP(i,j) dp[(data.size()*i)+j]

  //----------------------------------------------------------
  DbScan(vector<Point>& _data ,double _eps,int _mnpts, int verb=0):data(_data) {

    //labels = new int[data.size()];
    for(unsigned int i=0;i<data.size();i++) {
      //labels[i]=NOT_VISITED;
      labels.push_back(-99); //NOT_VISITED ¿?
    }
    eps=_eps;
    min_npts=_mnpts;
    verbosity = verb;
    numClasses=-1;

    //calculating distances 
    dp = new double[data.size()*data.size()];
    for(unsigned int i=0;i<data.size();i++) {
      for(unsigned int j=i; j<data.size();j++) {
	if(i==j)
	  DP(i,j)=0;
	else{
	  Point pa = data[i];
	  Point pb = data[j];
	  double dist=sqrt(pow(pa.x-pb.x,2) + PENALITZACIO_Y*pow(pa.y-pb.y,2));
	  DP(i,j)=dist;
	  DP(j,i)=dist;
	}
      }
    }
  }

  //----------------------------------------------------------
  ~DbScan(){
    delete dp;
    //delete labels;
  }

  //----------------------------------------------------------
  void setDist(double dist){
    eps=dist;
  }
  //----------------------------------------------------------
  void run() {
    
    for(unsigned int i=0;i<data.size();i++) {
      if(labels[i]==NOT_VISITED){
	vector<int> neighbours = regionQuery(i);
	if(neighbours.size()<min_npts) {
	  labels[i]=NOISE;
	}else {
	  numClasses++;
	  expandCluster(i,neighbours);
	}
      }
    }
  }
  //----------------------------------------------------------
  void run(int classToSplit) {
    for(unsigned int i=0;i<data.size();i++) {
      if (labels[i]==classToSplit)
	labels[i]=NOT_VISITED; 
    }

    for(unsigned int i=0;i<data.size();i++) {
      if(labels[i]==NOT_VISITED){
	vector<int> neighbours = regionQuery(i);
	if(neighbours.size()<min_npts) 
	  labels[i]=NOISE;       
	else {
	  numClasses++;	       	  
	  expandCluster(i,neighbours);
	}	
      }
    }
  }
  
  //----------------------------------------------------------
  vector<int> regionQuery(int p) {
    vector<int> res;
    for(unsigned int i=0;i<data.size();i++) {
      if (DP(p,i)<=eps)
	res.push_back(i);
    }
        
    return res;
  }

  //----------------------------------------------------------
  void expandCluster(int p,vector<int> neighbours)  {
    labels[p]=numClasses;
    for(unsigned int i=0;i<neighbours.size();i++) {
      if(labels[neighbours[i]]==NOT_VISITED || labels[neighbours[i]]==NOISE) {
	labels[neighbours[i]]=numClasses;
	vector<int> neighbours_p = regionQuery(neighbours[i]);
	if (neighbours_p.size() >= min_npts) {
	  expandCluster(neighbours[i],neighbours_p);
	}
      }
    }
  }
  //----------------------------------------------------------

  vector< vector<Point> > getClasses(){
    vector<Point> *classPoints= new vector<Point>[numClasses+1];
  
    for(unsigned int i=0;i<data.size();i++) {
      int label=labels[i];
      if (label > NOISE)
	classPoints[label].push_back( Point(data[i].x, data[i].y)); 
    }

    vector< vector<Point> > lines;
    for (int i = 0; i <= numClasses; ++i){
      if (classPoints[i].size() > 0){
	std::sort(classPoints[i].begin(), classPoints[i].end(), sort_points_func);       
	lines.push_back(classPoints[i]);
      }
    }
    std::sort(lines.begin(), lines.end(), sort_lines_func);
    delete [] classPoints;
    return lines;
  }
};

//----------------------------------------------------------
//-----------end of the DbScan class------------------------
//----------------------------------------------------------

void plotPoints(DbScan & dbscan, Mat grouped){    

  for(unsigned int i=0;i<dbscan.data.size();i++) {
    
    int label=dbscan.labels[i];

    cout << label << endl;
    cout << dbscan.data[i]<< endl;
    int point_shape=2;
    Scalar color(255,0,0);

    if(label>NOISE){ // Not noise
      color=colors[label%numColors];
      point_shape=2;
    } else { //noise
      point_shape=4; //diamond 
      color=Scalar(128,128,128); //grey
    }
           
    circle( grouped, dbscan.data[i], 5, color, point_shape );  
  }
}

//----------------------------------------------------------
void plotPoints(vector<vector<vector<Point> > > & lines, Mat grouped){
    
  int point_shape=2;
  Scalar color(255,0,0);
  for (uint r = 0; r < lines.size(); r++) { 
    for(uint l=0; l<lines[r].size(); l++) {
      for (uint p = 0; p < lines[r][l].size(); p++) {
	color=colors[l % numColors];
	circle( grouped, lines[r][l][p], 2, color, point_shape );  
      }
    }
  }
}

//----------------------------------------------------------
void plotRegions(Mat & img,vector< vector<Point> > & regions ){
  int thickness=2;
  int lineType = 8;
  bool isClosed=false;
  
  for (uint r=0; r <regions.size(); r++) {
    for (uint p = 0; p < regions[r].size(); p++) {
      Point startPoint = regions[r][p];
      Point endPoint   = regions[r][(p+1)% regions[r].size()];
      line(img, startPoint, endPoint,colors[r%numColors], thickness,lineType,isClosed);
    }
  }
}


//----------------------------------------------------------
void plotEllipseNeighbourhood(DbScan & dbscan, Mat img, std::map<int, int> & dist  ){
  for(uint i=0;i<dbscan.data.size();i++) {
    
    int label=dbscan.labels[i];
    Scalar color;

    if(label>NOISE){ // Not noise
      color=colors[label%numColors];
    } else { //noise
      color=Scalar(128,128,128); //grey
    }

    ellipse(img, dbscan.data[i], Size(dist[label]/2, dist[label]/2/PENALITZACIO_Y), 0, 0, 360, color);
  }
}

//----------------------------------------------------------
void plotPolyLines(Mat grouped, vector<vector< vector<Point> > > lines, bool printPolyline=true){

  int thickness=2;
  int lineType = 8;

  for (uint r = 0; r < lines.size(); r++)
    for (uint lin = 0; lin < lines[r].size(); lin++) {
      if (lines[r][lin].size() == 0) continue;
      std::sort(lines[r][lin].begin(), lines[r][lin].end(), sort_points_func);
   
      if (!printPolyline){
	Vec4f paramLine;
	fitLine(Mat(lines[r][lin]), paramLine,DIST_WELSCH,0,0.01,0.01);
      
	float vx=paramLine[0];
	float vy =paramLine[1];
	float xx= paramLine[2];
	float yy=paramLine[3];
      
	if (vx != 0){
	  int y_ini = int(((lines[r][lin][0].x - xx) * vy/vx) + yy);
	  int y_fin = int(((lines[r][lin][lines[r][lin].size()-1].x - xx) * vy/vx) +yy);	
	  line(grouped, Point(lines[r][lin][0].x,y_ini), Point(lines[r][lin][lines[r][lin].size()-1].x,y_fin),colors[lin%numColors], thickness,lineType);
	}
      }else {
	for (uint l = 0; l < lines[r].size(); l++) {
	  if (lines[r][l].size() == 0) continue;
	
	  for (uint p = 0; p < lines[r][l].size()-1; p++) {
	    Point startPoint=lines[r][l][p];
	    Point endPoint=lines[r][l][p+1];
	  
	    line(grouped, startPoint, endPoint,colors[l%numColors], thickness,lineType);
	  }
	}
      }
    }
}
//----------------------------------------------------------
void plotPolyLines(Mat grouped, DbScan & dbscan){
  vector<Point> *pointsClasses= new vector<Point>[dbscan.numClasses+1];
  for(uint i=0;i<dbscan.data.size();i++) {
    int label=dbscan.labels[i];
    if (label > NOISE){ 
      pointsClasses[label].push_back( Point(dbscan.data[i].x, dbscan.data[i].y)); 
    }
  }

  int thickness=2;
  int lineType = 8;

  for (int i = 0; i <= dbscan.numClasses; i++) {
    if (pointsClasses[i].size() == 0) continue;
    vector<Point> classPoints=pointsClasses[i];
    std::sort(classPoints.begin(), classPoints.end(), sort_points_func);
    int dist=0;

    classPoints.insert(classPoints.begin(),Point(classPoints[0].x-dist, classPoints[0].y));
    if (classPoints[0].x < 0) classPoints[0].x = 0;
    classPoints.push_back(Point(classPoints[classPoints.size()-1].x+dist, classPoints[classPoints.size()-1].y));
    for (uint p = 0; p < classPoints.size()-1; p++) {
      Point startPoint=classPoints[p];
      Point endPoint=classPoints[p+1];

      line(grouped, startPoint, endPoint,colors[i%numColors], thickness,lineType);
    }
  }
  delete [] pointsClasses;
}

//---------------------------------------------------------------------
void purgeLines(vector< vector<Point> > & lines, uint numMinPoints){
  for (uint l = 0; l < lines.size(); l++) {
    if (lines[l].size() < numMinPoints){
      lines.erase(lines.begin() + l);
      l--;
    }
  }
}


//---------------------------------------------------------------------

void fussionLines(vector< vector<Point> > & lines_rotades, Mat & img,double maxAngleAllowed){
  if (lines_rotades.size() == 0 ) return;


  int xmin=INT_MAX, xmax=0;
  for (uint i = 0; i < lines_rotades.size(); i++) {
    if (xmin > lines_rotades[i][0].x)
      xmin = lines_rotades[i][0].x;
    if (xmax < lines_rotades[i][lines_rotades[i].size()-1].x)
      xmax = lines_rotades[i][lines_rotades[i].size()-1].x;
  }
  
  int sepMaxAcceptable = (xmax - xmin)/5;
  
  
  bool fussionades;
  int cont =0;
  do{
    fussionades=false;
    std::sort(lines_rotades.begin(), lines_rotades.end(), sort_linesX_func);
   
    // fussionem
    for (uint l = 0; l <lines_rotades.size() && !fussionades; l++) {      
      //tenim en compte nomes les puntes
     
      // distancia minima de la punta dreta a totes les puntes esquerres no solapades
      double dist, dist_min = DBL_MAX;
      int lin_mes_prox = -1;
      for (uint l2 = 0; l2 <lines_rotades.size(); l2++) {

	int origRigthX = lines_rotades[l][lines_rotades[l].size()-1].x;
	int targetLeftX = lines_rotades[l2][0].x;
	
	if ( l2 != l && origRigthX <  targetLeftX){
	  double dx = targetLeftX - origRigthX;
	  double dy = lines_rotades[l][lines_rotades[l].size()-1].y -  lines_rotades[l2][0].y;
	  
	  dist = sqrt(dx*dx + 10*PENALITZACIO_Y* dy*dy);
	  
	  if (dist_min > dist){
	    dist_min = dist;
	    lin_mes_prox = l2;
	  }
	
	}
      }
      
      if (dist_min < sepMaxAcceptable){	
	lines_rotades[l].insert(lines_rotades[l].end(), lines_rotades[lin_mes_prox].begin(), lines_rotades[lin_mes_prox].end());	

	//esborra la linia sobrant
	lines_rotades.erase(lines_rotades.begin()+lin_mes_prox);

       	fussionades=true;	
          
	cont++;
      }
    }
  }while(fussionades);
  
}


//----------------------------------------------------------
bool * isALeaf(DbScan & dbscan, double maxAngAlowed, int thresDist){
  
  vector<Point> *pointsClasses= new vector<Point>[dbscan.numClasses+1];
  for(uint i=0;i<dbscan.data.size();i++) {
    int label=dbscan.labels[i];
    if (label != NOISE){ 
      pointsClasses[label].push_back( Point(dbscan.data[i].x, dbscan.data[i].y)); 
    }
  }

  bool * res = new bool [dbscan.numClasses+1];

  for (int clas = 0; clas <= dbscan.numClasses; clas++) {
    res[clas]=true; //by default is a leaf
    
    if (pointsClasses[clas].size() == 0) {continue;}

    double y_promig = 0;
    //double length_horiz = 0;
    
    vector<Point> classPoints=pointsClasses[clas];
    std::sort(classPoints.begin(), classPoints.end(), sort_points_func);
   
    for (uint p = 0; p < classPoints.size()-1; p++) {
      Point startPoint= classPoints[p];
      Point endPoint=classPoints[p+1];

      double a=( atan((double)( endPoint.y - startPoint.y )/( endPoint.x - startPoint.x )));
      if (a < 0) a=-a;
      if ((a*180)/M_PI < maxAngAlowed){
        //length_horiz += sqrt(pow(startPoint.y - endPoint.y,2) + pow(startPoint.x - endPoint.x,2));
	y_promig += endPoint.y + startPoint.y ;
      }
    }
    //length_horiz /=  (classPoints.size()-1) <= 0? DBL_MAX:(classPoints.size()-1);
    
    for (uint p = 0; p < classPoints.size()-1; p++) {
      Point startPoint= classPoints[p];
      Point endPoint=classPoints[p+1];

      double a=( atan((double)( endPoint.y - startPoint.y )/( endPoint.x - startPoint.x )));
      if (a < 0) a=-a;

      if ((a*180)/M_PI > maxAngAlowed){
	
	double length_vert=sqrt( pow(startPoint.y-endPoint.y,2)+pow(startPoint.x-endPoint.x,2));
        if (length_vert > thresDist){ //length_horiz*0.8 ){         
          res[clas]=false;              
	} else{  //fals positiu

	  //decidim quin dels dos punts eliminar
          int posterior=0;
          if (abs(startPoint.y - y_promig) < abs(endPoint.y - y_promig)){            
            posterior=1;
          }
	  
          unsigned int pos=0;
          while (pos < dbscan.data.size() && dbscan.data[pos] !=  classPoints[p+posterior] ){  //trobar el punt en dades     
            pos++;
          }
	  // eliminem el punt
	  dbscan.labels[pos]=NOISE;
	  p++;
        }
      }
    }
  }
  return res;
    
}


//--------------------------------------------------------------------
vector< vector<Point> > getLines(vector<Point> rotated_points,int dist,int dens,int decrDist,int maxAngleAllowed, int thresholdDist, int verbosity, string & inFileName, string & outFileName){
 static int iter=0;
  
  DbScan * dbscan= new DbScan(rotated_points,dist,dens,verbosity);
  dbscan->run();

  //posem la distancia per a cada classe
  std::map<int, int> distClasses;
  for (int i = 0; i <= dbscan->numClasses ; i++) {
    distClasses[i]=dist;
  }

  
  if (verbosity == 2 && inFileName.size()!=0 && outFileName.size()!=0){
      Mat img_tmp = imread(inFileName);  // caldria usar la imatge rotada.
      plotPoints(*dbscan, img_tmp);
      plotEllipseNeighbourhood(*dbscan, img_tmp, distClasses);

      std::stringstream iteracio;
      iteracio << iter;
      cerr << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
      try{
    	imwrite(outFileName+"_iteracio_"+iteracio.str()+".png",img_tmp);
      }catch (cv::Exception e){
    	cout << "ERROR en " << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
      }
    }

  int numClassesAnt=dbscan->numClasses;
  bool repetir;
  //static int iter=0;
  bool novesClases = false;
  
  do {
    repetir=false;
    novesClases=false;
    bool * leaf = isALeaf(*dbscan, maxAngleAllowed,thresholdDist);
      
    for (int i = 0; i <= dbscan->numClasses; i++) {
      if (!leaf[i]){ 
	dbscan->setDist(distClasses[i]-decrDist);
	dbscan->run(i);
	
	//reduim la distancia de veins per a les noves classes creades
	for (int j = numClassesAnt+1; j <= dbscan->numClasses; j++) {	 	  
	   distClasses[j]=distClasses[i]-decrDist;
	}
		
	if (numClassesAnt +1 < dbscan->numClasses)
	  novesClases=true;
	
	numClassesAnt=dbscan->numClasses;
	repetir=true;
	break;
      }
    }

    iter++;
    
    // if (verbosity == 2 && novesClases && inFileName.size()!=0 && outFileName.size()!=0){
    //   Mat img_tmp = imread(inFileName);  // caldria usar la imatge rotada.
    //   plotPoints(*dbscan, img_tmp);
    //   plotEllipseNeighbourhood(*dbscan, img_tmp, distClasses);

    //   std::stringstream iteracio;
    //   iteracio << iter;
    //   cerr << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
    //   try{
    // 	imwrite(outFileName+"_iteracio_"+iteracio.str()+".png",img_tmp);
    //   }catch (cv::Exception e){
    // 	cout << "ERROR en " << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
    //   }
    // }
    
    if (verbosity == 3 && novesClases && inFileName.size()!=0 && outFileName.size()!=0){
      Mat img_tmp = imread(inFileName);
      plotPoints(*dbscan, img_tmp);
      plotPolyLines(img_tmp,*dbscan);
      std::stringstream iteracio;
      iteracio << iter;
      cerr << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
      try{
    	imwrite(outFileName+"_iteracio_"+iteracio.str()+".png",img_tmp);
      }catch (cv::Exception e){
    	cout << "ERROR en " << "writing " << outFileName+"_iteracio_"+iteracio.str()+".png" << endl;
      }
    }

    delete [] leaf;
  }while (repetir);


  vector< vector<Point> > rotated_lines=dbscan->getClasses();
  delete dbscan;
  return rotated_lines;

}

//----------------------------------------------------------
void usage (char * programName){
 
  cerr << "Usage: "<<programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -p #String pointsFileName" << endl;
  cerr << "             -i #String imageFileName" << endl;
  cerr << "             -x #String xmlInputFileName" << endl;
  cerr << "             [-o #String] outputImageFileName (for demo)" << endl;
  cerr << "             [-d #int] reachability distance (by default automaticaly set)" << endl;
  cerr << "             [-n #int] density in point (default 2)"<< endl;
  cerr << "             [-D #int] decrDistStep (default 5)" << endl;
  cerr << "             [-a #int] max angle allowed (by default 60 degrees)" << endl;
  cerr << "             [-m #int] min num of points per baseline to be considered (by default 2)" << endl;
  //  cerr << "             [-s ] strait baseline (by default poliline)" << endl;
  cerr << "             [-N #int] max num of points per baseline"<< endl;
  cerr << "             [-v #int verbosity] " << endl;
}
//----------------------------------------------------------
int main(int argc,char** argv ) {
  string inFileName="", outFileName="",pointsFileName="", xmlFileName="";
  int dist=0, dens=2;
  int decrDist=5;
  int option;
  int maxAngleAllowed=60;
  int MIN_NUM_POINTS_LINE=2;
  int verbosity=0;
  bool printPolyline=true;
  int finalNumberOfPointsPerLine=-1;
  int thresholdDist=5;
  
  if(argc == 1){
    usage(argv[0]);
    return -1;
  }

  while ((option=getopt(argc,argv,"i:o:p:x:d:n:D:a:m:N:t:v:b"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 'o':
      outFileName = optarg;
      break;
    case 'p':
      pointsFileName = optarg;
      break;
    case 'x':
      xmlFileName = optarg;
      break;
    case 't':
      thresholdDist=atoi(optarg);
      break;
    case 'd':
      dist=atoi(optarg);
      break;
    case 'n':
      dens=atoi(optarg);
      break;
    case 'D':
      decrDist=atoi(optarg);
      break;
    case 'a':
      maxAngleAllowed=atoi(optarg);
      break;
    case 'm':
      MIN_NUM_POINTS_LINE = atoi(optarg);
      break;
    case 'b':
      printPolyline=false;
      break;
    case 'N':
      finalNumberOfPointsPerLine=atoi(optarg);
      break;
    case 'v':
      verbosity=atoi(optarg);
      break;
    default:
      usage(argv[0]);
      return(-1);
    }

  if (pointsFileName.size()==0){
    cerr << argv[0] << " Error: points file name must be provided" << endl;
    usage(argv[0]);
    return (-1);
  }

  if ((verbosity == 2 || verbosity == 3) && (inFileName.size()==0 || outFileName.size()==0))
    cerr << "WARNING: verbosity mode 4 needs an image file name and and output file name"<< endl;

  vector<Point> points;
  int maxDisX=getPointsFromFile(pointsFileName,points);

  Mat img;
  if (inFileName.size()!=0){
    img = imread(inFileName,0);
  }

 if (!img.data) {
    cerr << "ERROR reading the image file "<< inFileName<< endl;
    return -1;
  }

  // slopeClass rotator(img);
  // rotator.deslope();
  // double angle=rotator.getSlopeAngle();
    
  // //  Mat img_rotada=rotator.getRotada();

  
  // if (dist==0){
  //   dist=maxDisX;
  //   if (verbosity >= 1)
  // 	cerr << "Estimated meighbours dist = " << dist << endl;
  // }

  if (xmlFileName.size() <= 0){
    cerr << "ERROR: xml file name must been provided"<< endl;
    exit(-1);
  }
  pugi::xml_document page;
  pugi::xml_parse_result result = page.load_file(xmlFileName.c_str());
  if (!result){
    cerr << "ERROR: file: " << xmlFileName << " cannot not been opened" << endl;
    exit(-1);
  }
  
  vector <vector <cv::Point> >  regions= getRegions(page);

  vector< vector< vector<Point> > > rotated_lines;
  
  //si no hi han regions defindes
  if (regions.size() <= 0){
    vector<Point> r;
    r.push_back(Point(0,0));
    r.push_back(Point(img.cols-1,0));
    r.push_back(Point(img.cols-1,img.rows-1));
    r.push_back(Point(0,img.rows-1));   
   
    regions.push_back(r);

    // no regions given
    pugi::xml_node reg = page.child("PcGts").child("Page").append_child("TextRegion");
    pugi::xml_attribute id_attr = reg.append_attribute("id");
    id_attr.set_value("r0");
    pugi::xml_node reg_coords=reg.append_child("Coords");
    pugi::xml_attribute reg_points_attr = reg_coords.append_attribute("points");
    stringstream reg_coord;
    reg_coord << "0,0 " << img.cols-1 << ",0 " << img.cols-1 << "," <<  img.rows-1 << " 0," <<img.rows-1;
    reg_points_attr.set_value(reg_coord.str().c_str());

    
  }

 
  vector<Point> selected_points;
  vector< vector< vector<Point> > > lines_finals;
  vector<Point> rotated_points;
  
  for (uint r = 0; r < regions.size(); r++) {
    selected_points.clear();
    // seleccionem els punts dins de la regio.
    for (uint p = 0; p < points.size(); p++) {
      if(pointPolygonTest(regions[r], points[p], false) == 1)
	selected_points.push_back(points[p]);
    }

    int minX=img.cols;
    int maxX=0;
    int minY=img.rows;
    int maxY=0;
   
    for (uint p = 0; p < regions[r].size(); p++) {
      if (minX > regions[r][p].x)
        minX = regions[r][p].x;
      if (maxX <  regions[r][p].x)
        maxX = regions[r][p].x;
      if (minY > regions[r][p].y)
        minY = regions[r][p].y;
      if (maxY <  regions[r][p].y )
        maxY = regions[r][p].y;
    }
    if (minX < 0 ) minX = 0;
    if (minY < 0 ) minY = 0;
    if (maxX > img.cols ) maxX = img.cols;
    if (maxY > img.rows ) maxY = img.rows;
    
    // slopeClass rotator(img,Point(minX,minY),Point(maxX,maxY));
    slopeClass rotator(img, minX, minY,maxX,maxY);
    rotator.deslope();
    double angle=rotator.getSlopeAngle();


    //rotem els punts    
    rotated_points.clear();
    if (angle == 0)
      rotated_points = selected_points;
    else{
      for (uint p = 0; p < selected_points.size(); p++){ 
	rotated_points.push_back( rotatePoint(selected_points[p], -angle, maxY-minY, maxX-minX));
      
	//if (rotated_points[p].y < 0) rotated_points[p].y = 0;
	//	if (rotated_points[p].y >= img.rows) rotated_points[p].y = img.rows-1;
      }

    }
  
    // calculem la distancia per al clustering   
      dist = maxX - minX;

      vector< vector<Point> > rotated_lines = getLines(rotated_points, dist, dens, decrDist, maxAngleAllowed, thresholdDist, verbosity, inFileName, outFileName);
      
      purgeLines(rotated_lines,MIN_NUM_POINTS_LINE);
      fussionLines(rotated_lines,img,maxAngleAllowed);

      
      //des-rotem els punts      
      vector<Point> linia;
      vector< vector<Point> > lines;

      if ( angle == 0){
	 for (uint l = 0; l <rotated_lines.size(); l++){ 
	   lines.push_back(rotated_lines[l]);
	 }
      }else{
	 for (uint l = 0; l <rotated_lines.size(); l++){ 
     
	   linia.clear();
	   for (uint p = 0; p < rotated_lines[l].size(); p++) {
	     linia.push_back( rotatePoint(rotated_lines[l][p], angle,  maxY-minY, maxX-minX));
	     
	     if (linia[p].y < 0) linia[p].y = 0;
	     if (linia[p].y >= img.rows) linia[p].y = img.rows-1;
	   }

	   lines.push_back(linia);
	 }
       }
      lines_finals.push_back(lines);
    }

  //ordenem les linies
  for (int i = 0; i < lines_finals.size(); i++) {
      std::sort(lines_finals[i].begin(), lines_finals[i].end(),sort_lines_func);
  }

  //actualitzar xml
  updateXml(page,lines_finals,finalNumberOfPointsPerLine);

  page.save_file(xmlFileName.c_str());

  
  if (inFileName.size()!=0 && outFileName.size()!=0){
    Mat img_tmp = imread(inFileName,IMREAD_COLOR);    
    plotPoints(lines_finals,img_tmp);
    plotPolyLines(img_tmp,lines_finals,printPolyline);
    //plotRegions(img_tmp,regions );
    imwrite(outFileName,img_tmp);
  } 
}
  
