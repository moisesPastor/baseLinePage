#ifndef _LIB_XML_PAGE_H_
#define _LIB_XML_PAGE_H_

#include <vector>
#include <pugixml.hpp>
#include <opencv2/opencv.hpp>
using namespace std;

vector <vector <cv::Point > > getBaselines(pugi::xml_document & page);
void getBaselinesFromRegion(pugi::xml_node & text_region, vector< vector <cv::Point > > & baselines);
vector <vector <cv::Point > > getRegions(pugi::xml_document & page);

void getRegionCoords(pugi::xml_node & text_region, vector<cv::Point> & region);

void updateXml(pugi::xml_document & page,  vector< vector< vector<cv::Point> > > & lines_finals, int numPointsPerLine=-1);

#endif
