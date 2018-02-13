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
 * 
 * translate.cpp
 *
 *  Created on: 20/10/2017
 *      Author: Moisés Pastor i Gadea
 */

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>
#include "pugixml.hpp"

using namespace cv;
using namespace std;

const int numColors=36;
static Scalar colors[] = {
  Scalar(160, 130, 240), Scalar(80, 50, 160), Scalar(60, 60, 60), Scalar(30, 100, 200), Scalar(100, 150, 250), Scalar(50, 200, 230), Scalar(25, 10, 100), Scalar(30, 235, 140), Scalar(15, 150, 80), Scalar(50, 250, 90), Scalar(5, 130, 30), Scalar(160, 210, 50), Scalar(210, 150, 70), Scalar(245, 240, 70), Scalar(160, 150, 20), Scalar(240, 190, 50), Scalar(120, 90, 10), Scalar(90, 60, 10), Scalar(250, 150, 50), Scalar(160, 90, 5), Scalar(110, 70, 20), Scalar(250, 120, 30), Scalar(250, 80, 5), Scalar(250, 70, 10), Scalar(240, 60, 30), Scalar(120, 30, 10), Scalar(120, 15, 10), Scalar(250, 30, 20), Scalar(240, 15, 15), Scalar(100, 3, 3),Scalar(255, 0, 0), Scalar(0, 255, 0), Scalar(0, 0, 255), Scalar(255, 255, 0), Scalar(255, 0, 255), Scalar(0, 255, 255)
};

void drawPolyLines(Mat & img,vector< vector<Point> > & lines, int n_lin=-1 ){
  int thickness=2;
  int lineType = 8;
  bool isClosed=false;
  int point_shape=2;

  int l=0;
  int num_lines=lines.size();
  if (n_lin >= 0) { // para dibujar solo una linea
    l=n_lin;
    num_lines=n_lin+1;
  }

  for (; l <num_lines; l++) {
    for (uint p = 0; p < lines[l].size()-1; p++) {
      Point startPoint = lines[l][p];
      Point endPoint   = lines[l][p+1];
      circle(img, startPoint, 4, colors[l%numColors], point_shape );
      line(img, startPoint, endPoint,colors[l%numColors], thickness,lineType,isClosed);
    }
  }

}

//----------------------------------------------------------
vector <vector <cv::Point > > getBaselines(pugi::xml_document & page){
  vector< vector <cv::Point > > baselines;
  vector<cv::Point> tmp_baseline;

  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    
    for (pugi::xml_node line_region = text_region.child("TextLine"); line_region; line_region = line_region.next_sibling("TextLine")){   
      tmp_baseline.clear();
      std::vector<std::string> string_values;
      string point_string = line_region.child("Baseline").attribute("points").value();
      
      if(point_string != ""){

	istringstream point_stream(point_string);
	std::string string_point;
	while (point_stream >> string_point){
	  int cont_comas=0;
	  for (int i = 0; i < string_point.size(); i++) {
	    if (string_point[i] == ','){
	      string_point[i] = ' ';
	      cont_comas++;
	    }
	  }

	  if (cont_comas != 1){
	    cerr << "drawBaselines ERROR: regions cords bad format"<< endl;
	    exit(-1);
	  }	

	  int x,y;
	  istringstream(string_point) >> x >> y;;
	  tmp_baseline.push_back(cv::Point(x,y));
	}
	baselines.push_back(tmp_baseline);	  
      }      
    }
  }
  return baselines;
}

//----------------------------------------------------------
vector <vector <cv::Point > > getRegions(pugi::xml_document & page){
  vector< vector <cv::Point > > regions;
  vector<cv::Point> tmp_region;

  int n_reg=0;
  for (pugi::xml_node text_region = page.child("PcGts").child("Page").child("TextRegion"); text_region; text_region = text_region.next_sibling("TextRegion")){
    
    string point_string = text_region.child("Coords").attribute("points").value();
    
    if(point_string != ""){

      tmp_region.clear();
      istringstream point_stream(point_string);
      string string_point;
      while (point_stream >> string_point){
	int cont_comas=0;
	for (uint i = 0; i < string_point.size(); i++) {
	  if (string_point[i] == ','){
	    string_point[i] = ' ';
	    cont_comas++;
	  }
	}

	if (cont_comas != 1){
	  cerr << "drawBaselines ERROR: regions cords bad format"<< endl;
	  exit(-1);
	}

        int x,y;
        istringstream(string_point) >> x >> y;;
	tmp_region.push_back(cv::Point(x,y));
	
      }
      n_reg++;
      regions.push_back(tmp_region);
    }
  }
  return regions;
}



//----------------------------------------------------------
void usage (char * programName){

  cerr << "Usage: "<<programName << " options " << endl;
  cerr << "      options:" << endl;
  cerr << "             -i imageFileName" << endl;
  cerr << "             -x pointsFileName (XML PAGE format)" << endl;
  cerr << "            [ -l #int draw just this line]" << endl;
  cerr << "            [-o outputfile] " << endl;
}
//----------------------------------------------------------
int main(int argc,  char ** argv) {
 string inFileName="", outFileName="", xmlFileName="";
  int num_lin=-1; //vol dir totes les linies
  int option;

  if(argc == 1){
    usage(argv[0]);
    return -1;
  }

  while ((option=getopt(argc,argv,"i:o:x:l:v"))!=-1)
    switch (option)  {
    case 'i':
      inFileName = optarg;
      break;
    case 'o':
      outFileName = optarg;
      break;
    case 'x':
      xmlFileName = optarg;
      break;
    case 'l':
      num_lin=atoi(optarg);
      break;
    default:
      usage(argv[0]);
      return(-1);
    }

  if (inFileName.size()==0 || outFileName.size()==0 || xmlFileName.size()==0){
    cerr << argv[0] << " Error: input and output and xml file names must be provided" << endl;
    usage(argv[0]);
    return (-1);
  }
  
  Mat img=imread(inFileName);
  if (!img.data) {
    cerr << "ERROR: file "<< inFileName << " can not be oppened" << endl;
    exit(-1);
  }
  
  pugi::xml_document page;
  pugi::xml_parse_result result = page.load_file(xmlFileName.c_str());
  if (!result){
    cerr << "ERROR: file: " << xmlFileName << " cannot not been opened" << endl;
    exit(-1);
  }


  
  vector <vector <cv::Point> >  lines= getBaselines(page);
  

  drawPolyLines(img,lines,num_lin);
  if( outFileName.size() > 0)
    imwrite( outFileName, img);
 
  return 0;
}
