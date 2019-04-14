#ifndef SCHEDULER_H 
#define SCHEDULER_H
#include <vector>
#include <set>

#include "Road.h"
#include "Car.h"
#include "Cross.h"
#include "Answer.h"
#include "Graph.h"
using namespace std;

extern vector<Car> car_ve;
extern vector<Road> road_ve; 
extern vector<Cross> cross_ve;
extern const bool debug;

class Scheduler
{
private:
    //bool print;
    bool all_car_end;
    size_t car_finish;
    int cur_time;
    set<int> wait_set;
    vector<int> wait_cars;
    vector<int> pri_wait_cars;
    vector<int> dead_lock_union;
    vector<int> cross_id_inc;
    void scheduleCarsOnRoad(const int road_index,vector<vector<pair<int,int> > >& car_on_road, bool reverse);
    void scheduleCarsOnRoad(int road_index);
    void scheduleCarsOnCross(int cross_index);
    bool scheduleWaitCars(int cross_index, size_t road_index, vector<vector<pair<int,int> > >& car_on_road,bool reverse);
    void scheduleCarOnLane(int cross_index,int road_index,vector<pair<int,int> > & car_queue);
    int getPriority(int road_index,int cross_index,Car* car);
    int compare_priority(int cross_indxe,int road_index,int priority, bool reverse, bool prior);
    int compare_priority(int cross_indxe,int road_index,int priority, bool reverse);
    int collisionDetection(vector<vector<pair<int,int> > >& car_on_road,int priority);
    int collisionDetection(vector<vector<pair<int,int> > >& car_on_road,int priority,bool prior);
    int toRoad(int car_index,int road_index,vector<vector<pair<int,int> > >& car_on_road,bool reverse);
    void scheduleCarInGarage(vector<int>& start_cars);
    void scheduleCarInGarage(vector<int>& start_cars,int road_index,bool reverse);
    int getMostPriorityLane(vector<vector<pair<int,int> > >& car_on_road);
public:
    void printDeadLockUnion(){
        for(size_t i=0;i<dead_lock_union.size();i++){
            if(dead_lock_union[i]!=-1)cerr<<"("<<i<<","<<dead_lock_union[i]<<") ";
        }
        cout<<endl;
    }
    //void scheduleCars(vector<int>& start_cars, int cur_time);
    void scheduleCars(vector<int>& start_cars, vector<int>& prior_start_cars, int cur_time);
    void scheduleCars(int ct);
    int scheduleAllCars(vector<Answer>& answer_ve);
    bool scheduleOneCar(int car_index,int road_index,bool reverse);
    int getWaitCarSize(){
        return pri_wait_cars.size()+wait_cars.size();
    }
    void init(){
        all_car_end = false;
        cur_time = 0;
        wait_set.clear();
        wait_cars.clear();
        pri_wait_cars.clear();
        vector<int> t(2*road_ve.size(),-1);
        dead_lock_union.swap(t);
    }
    Scheduler();
    ~Scheduler(){};
};
#endif