#ifndef TXTHANDLER_H
#define TXTHANDLER_H
#include <vector>
#include <string>
#include <map>

using namespace std;
class Car;
class Road;
class Cross;
class Answer;

extern std::map<int,int> car_map;
extern std::map<int,int> road_map;
extern std::map<int,int> cross_map;
extern vector<Car> car_ve;
extern vector<Road> road_ve; 
extern vector<Cross> cross_ve;
class TxtHandler
{
private:
    /* data */
public:
    void getVectorFromChar(vector<int>& ve,char c[]);
    void getCarFromTxt(vector<Car>& car_ve, const string& path);
    void getRoadFromTxt(vector<Road>& road_ve, const string& path);
    void getCrossFromTxt(vector<Cross>& cross_ve, const string& path);
    void getAnswerFromTxt(vector<Answer>& answer_ve, const string& path);
    void writeAnswerToTxt(vector<Car>& car_ve, const string& path);
    void writeAnswerToTxt(vector<int>& car_index_vec, const string & path);
    void reMap(vector<Car>& car_ve, vector<Road>& road_ve, vector<Cross>& cross_ve);
    void logRoadStatus(int cur_time);
    void logOneDirectionCarStatus(vector<vector<pair<int,int> > >& car_on_road,ofstream& ofs);
    void logRoadStatusV2(int cur_time);
    void logRoadStatusV2(int cur_time,const string& filename);
    void logCar();
    void logOneWayCar(int road_index,int cur_time,bool reverse,ofstream& ofs);
};

#endif