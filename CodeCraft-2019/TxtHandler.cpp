#include "TxtHandler.h"
#include <fstream>
#include "Car.h"
#include "Road.h"
#include "Cross.h"
#include <cstdlib>
#include <string.h>
#include "Answer.h"
#include <queue>
#include <algorithm>
#include <ostream>


using namespace std;

struct CmpByCarID_2{
    bool operator()(int lhs, int rhs){
        return car_ve[lhs].getID()<car_ve[rhs].getID();
    }
}cbid_2;
/* struct CmpBy_CarID{
    bool operator()(Car lhs, Car rhs){
        return lhs.getID()<rhs.getID();
    }
}cb_car_id;
struct CmpBy_RoadID{
    bool operator()(Road lhs, Road rhs){
        return lhs.getID()<rhs.getID();
    }
}cb_road_id;
struct CmpBy_CrossID{
    bool operator()(Cross lhs, Cross rhs){
        return lhs.getID()<rhs.getID();
    }
}cb_cross_id; */

void TxtHandler::getVectorFromChar(vector<int>& ve,char c[]){
    int len=1;
    while(c[len]!=')')len++;
    char ct[len];
    for(int i=0;i<len-1;i++)ct[i]=c[i+1];
    ct[len-1]='\0';
    
    const char *separator = ",";
    char* p = strtok(ct,separator);
    while(p){
        ve.push_back(atoi(p));
        p=strtok(NULL,separator);
    }
    return;
}

void TxtHandler::getCarFromTxt(vector<Car>& car_ve, const string& path){
    ifstream ifs;
    ifs.open(path,ifstream::in);
    char c[256];
    while(ifs.good()){
        ifs.getline(c,256);
        if(c[0]=='('){
            vector<int> ve;
            getVectorFromChar(ve,c);
            bool prior = ve[5]==1?true:false;
            bool preset = ve[6]==1?true:false;
            Car myCar = Car(ve[0],ve[1],ve[2],ve[3],ve[4],prior,preset);
            car_ve.push_back(myCar);
            car_map[ve[0]] = car_ve.size() - 1;
        }
    }
    ifs.close();
}

void TxtHandler::getRoadFromTxt(vector<Road>& road_ve, const string& path){
    ifstream ifs;
    ifs.open(path,ifstream::in);
    char c[256];
    while(ifs.good()){
        ifs.getline(c,256);
        if(c[0]=='('){
            vector<int> ve;
            getVectorFromChar(ve,c);
            road_ve.push_back(Road(ve[0],ve[1],ve[2],ve[3],ve[4],ve[5],ve[6]==1?true:false));
            road_map[ve[0]] = road_ve.size() - 1;
        }
    }
    ifs.close();
}

void TxtHandler::getCrossFromTxt(vector<Cross>& cross_ve, const string& path){
    ifstream ifs;
    ifs.open(path,ifstream::in);
    char c[256];
    while(ifs.good()){
        ifs.getline(c,256);
        if(c[0]=='('){
            vector<int> ve;
            getVectorFromChar(ve,c);
            int r[4];
            for(int i=0;i<4;i++)r[i]=ve[i+1];
            cross_ve.push_back(Cross(ve[0],r));
            cross_map[ve[0]] = cross_ve.size() - 1;
        }
    }
    ifs.close();
}

void TxtHandler::writeAnswerToTxt(vector<Car>& car_ve, const string& path){
    ofstream ofs;
    ofs.open(path,ofstream::app);
    if(ofs.is_open()){
        for(size_t i=0;i<car_ve.size();i++){
            if(!car_ve[i].getPreset()){
                Car& car = car_ve[i];
                ofs <<"("<<car.getID()<<","<<car.getBestPlanTime()<<",";
                vector<int> path = car.getBestPath();
                if(!path.empty())ofs<<road_ve[path[0]].getID();
                for(size_t j=1;j<path.size();j++){
                    ofs << "," << road_ve[path[j]].getID();
                }
                ofs<<")"<<endl;
            }
        }
        ofs.close();
    }
}

void TxtHandler::writeAnswerToTxt(vector<int>& car_index_vec, const string & path){
    ofstream ofs;
    ofs.open(path,ofstream::app);
    if(ofs.is_open()){
        for(size_t i=0;i<car_index_vec.size();i++){
            Car& car = car_ve[car_index_vec[i]];
            ofs <<"("<<car.getID()<<","<<car.getBestPlanTime()<<",";
            vector<int> path = car.getBestPath();
            if(!path.empty())ofs<<road_ve[path[0]].getID();
            for(size_t j=1;j<path.size();j++){
                ofs << "," << road_ve[path[j]].getID();
            }
            ofs<<")"<<endl;
        }
        ofs.close();
    }
}

void TxtHandler::getAnswerFromTxt(vector<Answer>& answer_ve, const string& path){
    ifstream ifs;
    ifs.open(path,ifstream::in);
    char c[256];
    while(ifs.good()){
        ifs.getline(c,256);
        if(c[0]=='('){
            vector<int> ve;
            getVectorFromChar(ve,c);
            vector<int> path(ve.begin()+2,ve.end());
            for(size_t i=0;i<path.size();i++){
                path[i] = road_map[path[i]];
            }
            ve[0] = car_map[ve[0]];
            answer_ve.push_back(Answer(ve[0],ve[1],path));
        }
    }
    ifs.close();
}

void TxtHandler::reMap(vector<Car>& car_ve, vector<Road>& road_ve, vector<Cross>& cross_ve){
/*     sort(car_ve.begin(),car_ve.end(),cb_car_id);
    sort(road_ve.begin(),road_ve.end(),cb_road_id);
    sort(cross_ve.begin(),cross_ve.end(),cb_cross_id); */
    for(size_t i=0;i<car_ve.size();i++)car_map[car_ve[i].getID()]=i;
    for(size_t i=0;i<road_ve.size();i++)road_map[road_ve[i].getID()]=i;
    for(size_t i=0;i<cross_ve.size();i++)cross_map[cross_ve[i].getID()]=i;
    for(size_t i=0;i<car_ve.size();i++){
        Car& car = car_ve[i];
        int sid = car.getS_id();
        int eid = car.getE_id();
        int n_sid = cross_map[sid];
        int n_eid = cross_map[eid];
        car.setSID(n_sid);
        car.setEID(n_eid);
    }
    for(size_t i=0;i<road_ve.size();i++){
        Road& road = road_ve[i];
        int sid = road.getS_id();
        int eid = road.getE_id();
        int n_sid = cross_map[sid];
        int n_eid = cross_map[eid];
        road.setSID(n_sid);
        road.setEID(n_eid);
    }
    for(size_t i=0;i<cross_ve.size();i++){
        Cross& cross = cross_ve[i];
        const int* road_index = cross.getRoadID();
        int temp[4];
        for(int i=0;i<4;i++){
            if(road_index[i]==-1)temp[i]=-1;
            else temp[i] = road_map[road_index[i]];
        }
        cross.setRoadID(temp);
    }
}

void TxtHandler::logRoadStatus(int cur_time){
    ofstream ofs;
    for(size_t i=0;i<road_ve.size();i++){
        bool hasCar = false;
        for(const auto lane : road_ve[i].car_on_road){
            if(!lane.empty()){
                hasCar = true;
                break;
            }
        }
        if(!hasCar&&road_ve[i].getIs_two_way()){
            for(const auto lane : road_ve[i].car_on_road_r){
                if(!lane.empty()){
                    hasCar = true;
                    break;
                }
            }
        }
        if(hasCar){
            Road& road = road_ve[i];
            string filename = "../config/Road/";
            filename.append(std::to_string(road.getID()));
            filename.append(".txt");
            //cout<<filename<<endl;
            ofs.open(filename,ofstream::app);
            if(ofs.is_open()){
                Road& road = road_ve[i];
                ofs<<"cur time:"<<cur_time<<endl;
                logOneDirectionCarStatus(road.car_on_road,ofs);
                if(road.getIs_two_way()){
                    ofs<<"reverse"<<endl;
                    logOneDirectionCarStatus(road.car_on_road_r,ofs);
                }
                ofs<<"-------------------------------------------------"<<endl;
                ofs.close();
            }
        }
    }
}

void TxtHandler::logOneDirectionCarStatus(vector<vector<pair<int,int> > >& car_on_road,ofstream& ofs){
    for(size_t i=0;i<car_on_road.size();i++){
        if(!car_on_road[i].empty()){
            ofs<<"Lane "<<i<<" ";
            for(size_t j=0;j<car_on_road[i].size();j++){
                int wait = car_ve[car_on_road[i][j].first].getWait()?1:0;
                int spd = car_ve[car_on_road[i][j].first].getMax_spd();
                ofs<<"("<<car_ve[car_on_road[i][j].first].getID()<<","<<car_on_road[i][j].second<<","<<to_string(spd)<<","<<to_string(wait)<<") ";
            }
            ofs<<endl;
        }
    }
}

void TxtHandler::logCar(){
    vector<int> car_inc;
    for(size_t i=0;i<car_ve.size();i++){
        car_inc.push_back(i);
    }
    sort(car_inc.begin(),car_inc.end(),cbid_2);
    string filename = "../config/Car/log.txt";
    ofstream ofs;
    ofs.open(filename,ofstream::out);
    if(ofs.is_open()){
        for(size_t i=0;i<car_inc.size();i++){
            int pri = car_ve[car_inc[i]].getPrior()?1:0;
            ofs<<to_string(pri)<<" "<<to_string(car_ve[car_inc[i]].getID())<<" "<<to_string(car_ve[car_inc[i]].getOnRoadTime())
                <<" "<<to_string(car_ve[car_inc[i]].getFinishTime())<<endl;
        }
        ofs.close();
    }
}

void TxtHandler::logRoadStatusV2(int cur_time){
    ofstream ofs;
    string filename = "../config/Road/1_road_status.txt";
    ofs.open(filename,ofstream::app);
    if(ofs.is_open()){
        for(size_t i=0;i<road_ve.size();i++){
            logOneWayCar(i,cur_time,false,ofs);
            if(road_ve[i].getIs_two_way())logOneWayCar(i,cur_time,true,ofs);
        }
        ofs.close();
    }
}

void TxtHandler::logRoadStatusV2(int cur_time,const string& name){
    ofstream ofs;
    string filename = "../config/Road/" + name;
    ofs.open(filename,ofstream::app);
    if(ofs.is_open()){
        for(size_t i=0;i<road_ve.size();i++){
            logOneWayCar(i,cur_time,false,ofs);
            if(road_ve[i].getIs_two_way())logOneWayCar(i,cur_time,true,ofs);
        }
        ofs.close();
    }
}

void TxtHandler::logOneWayCar(int road_index,int cur_time,bool reverse,ofstream& ofs){
    vector<vector<pair<int,int> > >& car_on_road = reverse?road_ve[road_index].car_on_road_r:road_ve[road_index].car_on_road;
    for(size_t i=0;i<car_on_road.size();i++){
        ofs<<to_string(road_ve[road_index].getID())<<","<<to_string(cur_time)<<","<<to_string(i)<<",";
        for(size_t j=0;j<car_on_road[i].size();j++){
            ofs<<to_string(road_ve[road_index].getLen()-car_on_road[i][j].second)<<","<<to_string(car_ve[car_on_road[i][j].first].getID())<<",";
        }
        ofs<<endl;
    }
}