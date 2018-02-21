// g++ -o extrauPoligons extrauPoligonsV19.cc Graf.cc slopeClass.cc `pkg-config opencv --cflags --libs` -I.

#include <unistd.h>

#include <map>
#include <sstream>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <stack> 
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>

//#include "mfSetClass.h"
#include "Graf.h"
#include "slopeClass.h"
#include "pugixml.hpp"

using namespace cv;
using namespace std;

int verbosity=0;
bool demo=false;
int MIN_NUM_POINTS_LINE=4;  //VORE QUE FER AMB AÇO


//---------------------------------------------------------------------
struct  k{
  bool operator() (cv::Point pt1, cv::Point pt2) { if(pt1.x < pt2.x) return true; else if (pt1.x > pt2.x) return false; else return pt1.x < pt2.y ;}
} sort_points_func;

//---------------------------------------------------------------------
struct k1{
  bool operator() (vector< Point > v1, vector < Point > v2){
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

//-----------------------------------------------------------
class ys{
public:
  int y_min,y_max;
  ys(int y1, int y2){
    y_min=y1; 
    y_max=y2;
  }
  ys(){}
};


//----------------------------------------------------------
inline int calc_y(Point startPoint, Point endPoint, int x){

  // si hi han trams verticals
  if (startPoint.x == endPoint.x) {
    int new_y=(endPoint.y + startPoint.y)/2;
    if (new_y >= 0 ) return new_y;
    else return -new_y;
  }

  float slope= float(endPoint.y-startPoint.y)/(endPoint.x-startPoint.x);
  float desp = startPoint.y - slope * startPoint.x;

  float ytemp = slope * x + desp;
  int y = slope*x + desp;
  if(ytemp-y>0.5) // round the value of y to the closest integer
    y++;
  return y;
}

//---------------------------------------------------------------------
Point rotatePoint(Point p,double angle,Mat & img){
  angle=angle*M_PI/180.0;

  double x = p.x - img.cols/2;
  double y = -p.y + img.rows/2;
  //cout << "("<<p.x << ","<< p.y <<") -> ";
  double x1 = x*cos(angle) - y*sin(angle);
  double y1 = x*sin(angle) + y*cos(angle);

  x =  (int)(x1+0.5) + img.cols/2;
  y = (int)(-y1-0.5) + img.rows/2;

  return Point(x,y);
}
//----------------------------------------------------------
void loadPolyLines(string linesFileName,vector< vector<Point> > & lines){


  ifstream linesFile;
  linesFile.open(linesFileName.c_str());
  if(!linesFile){
    cerr << "Error: File \""<< linesFileName <<  "\" could not be open "<< endl;
    exit(-1);
  }

  float x,y;
  int clas;

  const int BUFFSIZE = 120;
  char buff[BUFFSIZE];
  stringstream line;
  int c=0;
  do{
    line.str(""); // per a buidar line
    linesFile.getline( buff,  BUFFSIZE );
    line << buff;
    if (c++ > 100) { 
      cerr << "ERROR: Point header file format wrong!!" << endl;
      exit (-1);
    }
  }while ('#' == line.peek() || line.str().size() == 0);

  int numLines;
  line >> numLines;
 
  //data=new vector<vector<Point>>(num_lines);
  int cont_line=0;
  while (cont_line < numLines){
   int c=0;
   do{
     line.str(""); // per a buidar line
     line.clear();
     linesFile.getline( buff,  BUFFSIZE );
     line << buff;
     if (c++ > 100) { 
       cerr << "ERROR: Point file format wrong!!" << endl;
       exit (-1);
       }
   }while ('#' == line.peek() || line.str().size() == 0);

   int numPoints;
   line >> numPoints;
   if (numPoints <=0){
     std::cerr<< "Warning: line "<< cont_line << " with " << numPoints << " points" << std::endl;
     continue;
   }

   int cont_points=0;
   vector<Point> points;
   while (cont_points < numPoints) { 
     int c=0;
     do{
       line.str(""); // per a buidar line
       line.clear();
       linesFile.getline( buff,  BUFFSIZE );
       line << buff;
       if (linesFile.eof()){
	 cerr << "ERROR Not enought number of points "<< cont_points << " and it is say to be " << numPoints << endl;
	 exit(-1);
       }
       if (c++ > 1000) { 
	cerr << "ERROR: Point file format wrong!!" << endl;
	exit (-1);
       }
     }while ('#' == line.peek() || line.str().size() == 0);

     line >> x >> y >> clas;
     points.push_back(Point(x,y));
     cont_points++;
   }

   cont_line++;
   if (points.size() >= MIN_NUM_POINTS_LINE){
     lines.push_back(points);  
   }
  }
}




//----------------------------------------------------------
vector<Point>  millorCami(Mat & rotada, vector< vector<Point> >  & lines_rotades){
  //marges rotats. Valors respecte al eix de coord de la rotada
  map<int,ys> marges;

  int p_central=0, p_down=0;
  for (int x = 0; x < rotada.cols; x++) {
    if (p_central < lines_rotades[0].size()-2 && x >= lines_rotades[0][p_central+1].x) p_central++;
    if (p_down < lines_rotades[1].size()-2 &&x >= lines_rotades[1][p_down+1].x) p_down++;
   
    int y_central=calc_y(lines_rotades[0][p_central],lines_rotades[0][p_central+1],x);
    int y_down   =calc_y(lines_rotades[1][p_down],   lines_rotades[1][p_down+1],x);

    if (y_central >= rotada.rows) y_central=rotada.rows-1;
    if (y_down >= rotada.rows) y_down=rotada.rows-1;

    //if crossed
    if (y_central < y_down)
      marges[x]=ys(y_central,y_down);
    else 
      marges[x]=ys(y_down,y_central);
      
    if (marges[x].y_min + 15 > marges[x].y_max){
      if (marges[x].y_max + 15 < rotada.rows)
	marges[x].y_max = marges[x].y_max + 15;
      else {
	marges[x].y_min -= 15 - (rotada.rows - marges[x].y_max);
	marges[x].y_max = rotada.rows -1;
      }
    }

     // if (y_central > y_down) cerr << "Warning y_central = " << y_central << " y_down = " <<y_down<< endl;

    if (demo){
      //circle(rotada, Point(x,y_central), 2, Scalar(100), 2);
      //circle(rotada, Point(x,y_down), 2, Scalar(100), 2);
      circle(rotada, Point(x,marges[x].y_min),1, Scalar(100),2);
      circle(rotada, Point(x,marges[x].y_max), 1, Scalar(100), 2);
    }
  }


  if (marges.size()!= rotada.cols)
    cerr << "WARNING: size of marges structure must be "  << rotada.cols << " and is " << marges.size() << endl;

  Graf * graf=new Graf(rotada.cols*rotada.rows);
  for (int x = 0; x < rotada.cols-1; x++) {

    int y_up= marges[x].y_min;
    int y_down= marges[x].y_max;
    int marge=(int)(y_down-y_up)*0.15 + 0.5;
    //marge=0;
    for (int y = y_up+1; y< y_down-marge; y++) {
      int punt=y*rotada.cols + x;
      
      int punt_sud=(y+1)*rotada.cols + x; 
      int punt_nord=(y-1)*rotada.cols + x; 
      int punt_est=y*rotada.cols + x + 1; 
      int punt_sud_est=(y+1)*rotada.cols + x + 1;
      int punt_nord_est=(y-1)*rotada.cols + x + 1; 
      
      int pes_nord=255-rotada.at<uchar>(y,x)+ 255 - rotada.at<uchar>(y-1,x);
      int pes_sud=255-rotada.at<uchar>(y,x)+ 255 - rotada.at<uchar>(y+1,x);
      int pes_est=255-rotada.at<uchar>(y,x)+ 255 -rotada.at<uchar>(y,x+1);
      int pes_sud_est=255-rotada.at<uchar>(y,x) +  255-rotada.at<uchar>(y+1,x+1);
      int pes_nord_est=255-rotada.at<uchar>(y,x) + 255-rotada.at<uchar>(y-1,x+1);


      if (! graf->addEdge(punt, punt_nord, pes_nord) && verbosity > 1){
	cout << "NORD "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "NORD "<< punt_nord << " ("<< punt_nord %rotada.cols << "," << punt_nord/rotada.cols << ")"<< endl << endl;
      }
    
      if (! graf->addEdge(punt, punt_nord_est, pes_nord_est) && verbosity > 1 ){
	cout << "NORD-EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "NORD-EST "<< punt_nord_est << "("<< punt_nord_est %rotada.cols << "," << punt_nord_est/rotada.cols << ")"<< endl << endl;
      }

      if (! graf->addEdge(punt, punt_est,pes_est+5) && verbosity > 1 ){
	cout << "EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "EST "<< punt_est << " ("<< punt_est %rotada.cols << "," << punt_est/rotada.cols << ")"<< endl << endl;
      }
      
      if (! graf->addEdge(punt,punt_sud_est, pes_sud_est+15) && verbosity > 1 ){
	cout << "SUD-EST "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "SUD-EST "<< punt_sud_est << " ("<< punt_sud_est %rotada.cols << "," << punt_sud_est/rotada.cols << ")" << endl << endl;
      }

      if (! graf->addEdge(punt,punt_sud, pes_sud+20) && verbosity > 1 ){
	cout << "SUD "<< punt << " ("<< punt%rotada.cols << "," << punt/rotada.cols << ")"<< endl;
	cout << "SUD "<< punt_sud << " ("<< punt_sud %rotada.cols << "," << punt_sud/rotada.cols << ")" << endl << endl;
      }   
    
    }
  }

  //dijkstra per trobar el camí més curt
  int y_ini=( marges[0].y_max - marges[0].y_min )/4 + marges[0].y_min ;
  //si al rotar fora la vora negra
  int x_ini=0;
  uchar pixel= rotada.at<uchar>(x_ini,y_ini) ;
  while (x_ini && pixel == 0) 
    x_ini++;

  int orig=y_ini* rotada.cols; //+ x_ini;
  graf->dijkstra(orig);

  int * path=graf->getPath();
  double * dist=graf->getDist();
  
  if (path== NULL || dist==NULL) return vector<Point>();

  //per a tots els finals busquem el més barat:

  //cerr<< "marges finals " << marges[rotada.cols-1].y_min << " " << marges[rotada.cols-1].y_max << endl;

  double mesBarat=DBL_MAX;
  int pos=0;
  for (int y =  marges[rotada.cols-1].y_min; y < marges[rotada.cols-1].y_max; y++){
    if (dist[rotada.cols*(y+1) -1] <= mesBarat){
      mesBarat=dist[rotada.cols*(y+1) -1];
      pos=rotada.cols*(y+1) -1;
    }
  }
  if (verbosity > 1)
    cout << "Cami mes barat, cost -> " << mesBarat << endl;
  
  //recuperar el cami    
  //vector<Point> cami(rotada.cols);
  stack<Point> cami;
  vector<Point> cami_tornar(0);  //comença el vector amb 0 posicions. Per a que el size tinga algun sentit
  int x_ant=rotada.cols;
  if (mesBarat < DBL_MAX){
    while(path[pos] != orig && path[pos]!= -1){
      int x=pos%rotada.cols;
      int y=pos/rotada.cols;

      if (x != x_ant && x < rotada.cols){
	cami.push( Point(x,y));
	x_ant=x;
      }
      pos=path[pos];
    }

    x_ant--;
    while (x_ant >=0){
      cami.push( Point(x_ant,orig/rotada.cols));

      x_ant--;
    }
  }


  delete graf;

  while (!cami.empty()) {
    cami_tornar.push_back(cami.top());
    cami.pop();
  }


  return cami_tornar;
}

//----------------------------------------------------------
bool getLineImage(vector< vector<Point> > & lines,Mat & img,vector< vector<Point> > & segmentacio , unsigned int lin_num){

  //obtenim les vores de la imatge a tallar
  int y_min=INT_MAX;
  int y_max=0;
  for (int i = 0; i < lines[lin_num].size(); i++) 
    if (y_min > lines[lin_num][i].y)
      y_min = lines[lin_num][i].y;
  
  for (int i = 0; i < lines[lin_num+1].size(); i++) 
    if ( y_max < lines[lin_num+1][i].y)
       y_max = lines[lin_num+1][i].y;

  if (lin_num == lines.size()-1) y_max=img.rows-1;

  if (y_min + 10 > y_max) return false;

  // using clone
  Mat linia=img(Rect(0,y_min, img.cols, y_max-y_min+1)).clone();
  /*
  std::stringstream iteracio;
  iteracio << lin_num;  
  string nom="kk_noRot."+iteracio.str()+".jpg";
  imwrite(nom,linia);
  */
  slopeClass rotator(linia);
  linia.release();
  rotator.deslope();
 
  
  double angle=rotator.getSlopeAngle();
  rotator.rotar(-angle);
  Mat rotada=rotator.getRotada();
  
 //rotem els punts
  vector< vector<Point> >lines_rotades(2);
  for (int l = 0; l <2; l++) 
    for (int p = 0; p < lines[lin_num + l].size(); p++) {
      lines_rotades[l].push_back(rotatePoint(lines[lin_num + l][p], -angle, img));
      lines_rotades[l][lines_rotades[l].size()-1].y -= y_min; //relatiu a la imatge retallada
      if (lines_rotades[l][p].y < 0) lines_rotades[l][p].y = 0;
      if (lines_rotades[l][p].y >= rotada.rows) lines_rotades[l][p].y = rotada.rows-1;
      if (lin_num+l == lines.size()-1) lines_rotades[l][p].y = rotada.rows-1; //piso per a l'ultima linia
    }

  for (int l = 0; l < 2; l++) { 
    //afegim punt inicial a la col 0 i final a numCols-1
     lines_rotades[l].insert(lines_rotades[l].begin(), Point(0,lines_rotades[l][0].y));
     if (lines_rotades[l][0].y < 0) lines_rotades[l][0].y = 0;
     if (lines_rotades[l][0].y >= rotada.rows) lines_rotades[l][0].y = rotada.rows-1;

     lines_rotades[l].push_back(Point(rotada.cols-1, lines_rotades[l][lines_rotades[l].size()-1].y));
     if (lines_rotades[l][lines_rotades[l].size()-1].y < 0) lines_rotades[l][lines_rotades[l].size()-1].y = 0;
     if (lines_rotades[l][lines_rotades[l].size()-1].y >= rotada.rows) lines_rotades[l][lines_rotades[l].size()-1].y = rotada.rows-1;
  }

  static vector<Point> cami_dalt;
  static vector<Point> cami_baix;

  if (cami_baix.size() > 0)
    cami_dalt=cami_baix;
  
  if (lin_num < lines.size()-1)
    cami_baix=millorCami(rotada, lines_rotades);

  if  (cami_baix.size() <= 0){
    cerr << "Cami de baix, not found in line "<< lin_num << endl;
    return false;
  } else 
    for (int i = 0; i < cami_baix.size(); i++) {
      cami_baix[i]=Point(rotatePoint(cami_baix[i],angle,rotada));
      cami_baix[i].y+=y_min; //relatiu a la imatge general
      if (cami_baix[i].y < 0) cami_baix[i].y=0;
    }
  
  segmentacio.push_back(cami_baix);

  return true;
}
//----------------------------------------------------------
void fussionLines(vector< vector<Point> > & lines, Mat & img){

//espacio interlinea promedio
  double media=0,media_total=0,ant=0;;
  for (int lin_num = 0; lin_num < lines.size()-1; lin_num++) {
    ant=media;
    for (int p = 0; p < lines[lin_num].size(); p++) {
      media+=lines[lin_num][p].y;
    }
    media=media/lines[lin_num].size();
    if (lin_num > 0)
      media_total+=(media - ant);
  }

  media_total=media_total/lines.size();


  bool crossed=false;

  for (int lin_num = 0; lin_num < lines.size()-1; lin_num++) {
    
    double media_central=0,media_down=0;

    for (int p = 0; p < lines[lin_num].size(); p++) {
      media_central+=lines[lin_num][p].y;
    }
    media_central/=lines[lin_num].size();

    for (int p = 0; p < lines[lin_num+1].size(); p++) {
      media_down+=lines[lin_num+1][p].y;
    }
    media_down/=lines[lin_num+1].size();

    //if (media_central + 10 > media_down){
    if (media_down - media_central < media_total/3){
      // if (verbosity){
      // 	cerr << "WARNING: lines "<< lin_num << " and " <<lin_num+1 << " crossed" << endl;
      // 	cerr << "media_central = " << media_central << " media_down = " << media_down<< endl;
      // 	cerr << "      size 1 " << lines[lin_num].size() << endl;
      // 	cerr << "      size 2 " << lines[lin_num+1].size() << endl;
      // }

      //afegix els punts de la segona linia a la primera
      lines[lin_num].insert(lines[lin_num].end(), lines[lin_num+1].begin(), lines[lin_num+1].end());
      std::sort(lines[lin_num].begin(), lines[lin_num].end(), sort_points_func);
      
      //esborra la linia sobrant
      lines.erase(lines.begin() + lin_num+1);
            
      // if (verbosity){
      // 	cerr <<"      Num de linies " << lines.size() << endl << endl;
      // 	cerr <<"      total  "<< lines[lin_num].size() << endl<< endl;
      // }
      lin_num--; // per a repetir la linia
      crossed=true;
      break; //tornem a procesarla
    }
  }

  if (crossed)
    fussionLines(lines,img);
}

void purgarSegmentacio(  vector< vector<Point> >segmentacio, Mat img){
  for (int c = 0; c < segmentacio.size();c++) {
      for (int i = 0; i < segmentacio[c].size()-1; i++) {
	if (segmentacio[c][i].x == segmentacio[c][i+1].x){
	  if (segmentacio[c][i].y < segmentacio[c][i+1].y)
	    segmentacio[c].erase(segmentacio[c].begin() + i);
	  else
	    segmentacio[c].erase(segmentacio[c].begin() + i+1);
	
	  i--;
	}
	    //cout << "     "<< i << " " << segmentacio[c][i] << segmentacio[c][i+1] << endl;
	
      }
      // if (segmentacio[c].size() != img.cols){
  // 	cout << "despres "<<c << " -> " << img.cols << " vs " << segmentacio[c].size() << endl;
  //     }
  // }
  
  // for (int c = 0; c < segmentacio.size();c++) {
  //   cout << c << " " ;
  //   for (int i = 0; i < segmentacio[c].size()-1; i++) {
  //     cout << segmentacio[c][i].x << " ";
  //   }
  //   cout<< endl;
  }
}

vector <vector <cv::Point > > getBaselines(pugi::xml_document & page){
  vector< vector <cv::Point > > baselines;
  vector<cv::Point> tmp_baseline;

  int n_reg=0;
  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region;
 text_region = text_region.next_sibling("TextRegion")){
    
    int n_lin=0;
    
    for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
      tmp_baseline.clear();
      std::vector<std::string> string_values;
      string point_string = line_region.child("Baseline").attribute("points").value();
      if(point_string != ""){
        boost::split(string_values,point_string,boost::is_any_of(" "),boost::token_compress_on);
        
        for(unsigned int p = 0; p < string_values.size();p++){
          std::vector<std::string> string_point;
          boost::split(string_point,string_values[p],boost::is_any_of(","),boost::token_compress_on);
          int x;
          istringstream(string_point[0]) >> x;
          int y; 
          istringstream(string_point[1]) >> y;
          tmp_baseline.push_back(cv::Point(x,y));
        }
        n_lin++;
        baselines.push_back(tmp_baseline);
      }
      
    }
    n_reg++;
  }
  return baselines;
}

void normaliza_traza(vector<Point> & baseline, int NumPuntsNuevo=10){
  vector<Point> puntos(baseline);
 
 if (baseline.size()<=0) { 
      cerr << "WARNING: baseline with "<< baseline.size()<< " points" << endl;
      return;
  }

  baseline.clear();

  // calculamos el vector de distancias desde el punto inicial a cualquier otro
  int DistEntrePuntos;
  double * D=new double[puntos.size()];
  D[0]=0;
  for (uint j = 1; j < puntos.size(); j++) {
    int DX=(puntos[j].x - puntos[j-1].x);
    int DY=(puntos[j].y - puntos[j-1].y);
    D[j]=D[j-1]+sqrt((double)DX*DX+DY*DY);
  }
  
  // si no se recibe, se distribuyen los puntos que ya hay equidistantemente
  if (NumPuntsNuevo == 0){ 
    NumPuntsNuevo=puntos.size();
  }

  DistEntrePuntos=D[puntos.size()-1]/(NumPuntsNuevo-1);
  
  // ponemos el punto inicial donde estaba el original
   baseline.push_back(puntos[0]);
  int n=1;
    
  for (int j = 1; j < NumPuntsNuevo-1; j++) {
    while (!((D[n-1] <= j*DistEntrePuntos) && (j*DistEntrePuntos <= D[n]))) n++;
    float C;
    if (D[n-1]==D[n]) C=1;
    else C=(j*DistEntrePuntos-D[n-1])/(D[n]-D[n-1]);
    
    float TX=puntos[n-1].x + (puntos[n].x - puntos[n-1].x)*C;
    float TY=puntos[n-1].y + (puntos[n].y - puntos[n-1].y)*C;
    
    Point p_nuevo(int(TX+0.5), int(TY+0.5));
    baseline.push_back(p_nuevo);
  }

  // ponemos el punto final donde estaba el original
  baseline.push_back(puntos[puntos.size()-1]);
}
//----------------------------------------------------------


void pintaLinies(Mat & img, bool deBatABat,  vector <vector <cv::Point> >  lines,vector< map<int,int> > seg_lines){
  int MARGE=80;
  //pinta el poligon al voltant de les frases
  int x_ini=0;
  int x_fi=img.cols-1;
    
    //per a la primera linia
     if (!deBatABat){
       x_ini=lines[0][0].x - MARGE;
      	if (x_ini < 0) x_ini=0;	

	x_fi=lines[0][lines[0].size()-1].x+MARGE;
      	if (x_fi > img.cols) x_fi=img.cols-1;
     }
     for (int x = x_ini; x <x_fi; ++x)
	  circle(img, Point(x,seg_lines[0][x]),1, Scalar(100),1);

     //per a la resta de la pagina
     x_ini=0;
     x_fi=img.cols-1;
     
    for (int lin = 1; lin < seg_lines.size(); lin++) {
      if (!deBatABat){
	x_ini=lines[lin][0].x < lines[lin-1][0].x ? lines[lin][0].x : lines[lin-1][0].x ;
	x_ini-=MARGE;
      	if (x_ini < 0) x_ini=0;

      	x_fi=lines[lin][lines[lin].size()-1].x > lines[lin-1][lines[lin-1].size()-1].x ? lines[lin][lines[lin].size()-1].x : lines[lin-1][lines[lin-1].size()-1].x;
	x_fi+=MARGE;
      	if (x_fi > img.cols) x_fi=img.cols-1;
      }
      
      for (int x = x_ini; x <x_fi; ++x)
	circle(img, Point(x,seg_lines[lin][x]),1, Scalar(100),1);
    
    }

}

//----------------------------------------------------------
void use (char * programName){

  cerr << "Usage: "<<programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -x #String; inputPageXmlFile" << endl;
  cerr << "             -i #String;  inputImageFile" << endl; 
  //cerr << "             [-l #int line to be extracted (by default all)]" << endl;
  cerr << "             [-o #String ] outputImageFile] " << endl;
  cerr << "             [-d ] demo (by default false)]" << endl;
  cerr << "             [-m #int ] (by default 4)] min num points per line" << endl;
  cerr << "             [-t ] segment from the col 0 to numCols (by default no)"<< endl;
  cerr << "             [-v #int ] level of verbosity (by default 0)]" << endl;

}

//----------------------------------------------------------
int main(int argc,char** argv ) {
  string inFileName="",xmlFileName="",outputFileName="";
  int option;
  //int line=-1; //means all lines must be extracted
  bool deBatABat=false;

  while ((option=getopt(argc,argv,"i:x:dm:o:tv:"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 'x':
      xmlFileName = optarg;
      break;
    case 'o':
      outputFileName = optarg;
      break;
    case 'd':
      demo=true;
      break;
    case 't':
      deBatABat=true;
      break;
    case 'm':
      MIN_NUM_POINTS_LINE = atoi(optarg);
      break;
    case 'v':
      verbosity=atoi(optarg);
      break;
    default:
      use(argv[0]);
      return(-1);
    }

  if (xmlFileName.size()==0){
    cerr << argv[0] << " Error: points file name must be provided" << endl;
    use(argv[0]);
    return (-1);
  }
 
  Mat img;
  if (inFileName.size()!=0){
    img = imread(inFileName,0);
  }
  if (!img.data) {
    cerr << argv[0] << ": ERROR: image file " << inFileName.c_str() << " do not contains data " << endl;
    return (-1);
  }

  pugi::xml_document page;
  pugi::xml_parse_result result = page.load_file(xmlFileName.c_str());
  if (!result){
    cerr << "ERROR: file: " << xmlFileName << " could not be opened" << endl;
    exit(-1);
  }

 vector <vector <cv::Point> >  lines= getBaselines(page);
  if (lines.size()<=0){
    cerr << "Warning: file "<< xmlFileName << " do not contains lines "<< endl;
    exit(-1);
  }

  //llevem el skew de la pagina
  slopeClass desquewer(img);
  desquewer.deslope();
  
  double angleSlope=desquewer.getSlopeAngle();
  desquewer.rotar(-angleSlope);
  Mat img_rotated=desquewer.getRotada();
  
  for (int l = 0; l < lines.size(); l++) { 
    //ordenem els punts d'esquerre a dreta
    std::sort(lines[l].begin(), lines[l].end(), sort_points_func);
  }  

  //rotem els punts
  for (int l = 0; l < lines.size(); l++) 
    for (int p = 0; p < lines[l].size(); p++) 
      lines[l][p]=rotatePoint(lines[l][p], -angleSlope, img);

  //ordenem les linies
  std::sort(lines.begin(), lines.end(), sort_lines_func);

  //fussionem  linies base  ARA NO CAL PERQUE PARTIM DE LINIS BASE
  fussionLines(lines, img_rotated);

  if (verbosity > 0)
    cout << "Number of lines " << lines.size() << endl;

  
 //afegim una linia artificial baix de tot per a poder tindre vora en la segmentacio
  vector<Point> ultimaLinia;
  ultimaLinia.push_back(Point(img.cols/4, img.rows-1));
  ultimaLinia.push_back(Point(img.cols/1.5, img.rows-1));
  lines.push_back(ultimaLinia);
 
  vector< vector<Point> >segmentacio(0);
  for (int l = 0; l < lines.size()-1; l++) {      
    if (verbosity > 1)
      cout << endl<< "processing line "<< l+1<< endl;
    
    getLineImage(lines, img_rotated, segmentacio, l);
    //if (!getLineImage(lines, img_rotated, segmentacio, l))
    //  cerr << "WARNING: Crossed lines !!! " <<l << " and " << l+1<< endl;
  }


  //de-rotem els punts
  // for (int l = 0; l < lines.size(); l++) 
  //   for (int p = 0; p < lines[l].size(); p++) 
  //     lines[l][p]=rotatePoint(lines[l][p], angleSlope,img);


  //afegim una linia inicial damunt de la primera i igual a ella
  vector <cv::Point> linia_zero;
  linia_zero.push_back(Point(segmentacio[0][0].x,0));
  linia_zero.push_back(Point(segmentacio[0][ segmentacio[0].size()-1].x, 0));
  
  segmentacio.insert(segmentacio.begin(), linia_zero);
  
  vector< map<int,int> > seg_lines;

  //LES X'S ES DESPLACEN AL ROTAR
  for (int l = 0; l < segmentacio.size(); l++){ 
    map<int,int> seg;
    for (int p = 0; p < segmentacio[l].size(); p++) {
      //Point punt(rotatePoint(segmentacio[l][p], angleSlope,img));
      //seg[punt.x]=punt.y;
      seg[segmentacio[l][p].x] = segmentacio[l][p].y;
     
    }
    
    // Point primer=rotatePoint(segmentacio[l][0], angleSlope,img);
    Point primer(segmentacio[l][0]);
    if (primer.x > 0){
      int x_ini=primer.x - 1;
      for (int x = x_ini; x >=0 ; x--)  
        seg[x]=primer.y;
    }

    //Point ultim=rotatePoint(segmentacio[l][segmentacio[l].size()-1], angleSlope, img);
    Point ultim(segmentacio[l][segmentacio[l].size()-1]);
    if (ultim.x < img.cols - 1){
      for (int x = ultim.x+1; x < img.cols; x++)
	seg[x]=ultim.y;
    }
    seg_lines.push_back(seg);   
  }
  
  //al rotar es creen discontinuitats en les x
  for (int l = 0; l < seg_lines.size(); l++)
    for (int x = 1; x < img.cols; ++x) {
      if(seg_lines[l].find(x) == seg_lines[l].end())
	seg_lines[l][x]=seg_lines[l][x-1];
    }
  
  //li llevem la extensio al nom de fitx d'entrada
  size_t found = inFileName.find_last_of(".");
  inFileName.erase(found,-1);

  if (demo){
    pintaLinies(img_rotated,deBatABat, lines, seg_lines);
    string demoFileName;
    if (outputFileName.size() >0)
      demoFileName=outputFileName;
    else
      demoFileName=inFileName+"_segmented.jpg";

    imwrite(demoFileName,img_rotated);
    
  } else {
    //li llevem la extensio al nom de fitx d'entrada

    for (uint seg = 0; seg < segmentacio.size()-1; seg++) {

      int xini=0;
      int xfi=img_rotated.cols-1;

      if (!deBatABat){
        //mana la linia base de baix 
        xini=lines[seg+1][0].x;
        if (xini < 0) xini=0;
        xfi=lines[seg+1][lines[seg+1].size()-1].x ;
        if (xfi > img_rotated.cols) xfi=img_rotated.cols-1;
      }

      int y_min= img_rotated.rows;
      int y_max = 0;

     
		  
      for (int x = xini; x < xfi && x< segmentacio[seg+1].size(); x++) {
	 if (y_min > seg_lines[seg][x]) y_min = seg_lines[seg][x];
	 if (y_max < seg_lines[seg+1][x]) y_max = seg_lines[seg+1][x];
      } 

      Mat img_lin(y_max-y_min+1, xfi-xini,  CV_8UC1, Scalar(255));
      
      for (int x = xini; x < xfi && x< seg_lines[seg+1].size(); x++) {
	if ( seg_lines[seg].find(x) != seg_lines[seg].end())
	  for (int y = seg_lines[seg][x]; y < seg_lines[seg+1][x]; y++) {
	    if ( y < img_rotated.rows){
	      img_lin.at<uchar>(y-y_min, x-xini) = img_rotated.at<uchar>(y,x);
	    }
	  }
      }


      std::stringstream num_linea_str;
      num_linea_str << seg;
      outputFileName=inFileName+"_"+num_linea_str.str()+".jpg";
     
      imwrite(outputFileName,img_lin);
      img_lin.release();
    }

  }
  
 
}
