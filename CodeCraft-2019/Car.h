#ifndef CAR_H
#define CAR_H
#include <iostream>
#include <queue>
#include <set>


using namespace std;
class Car{
public:
    Car(int id, int s_id, int e_id, int max_spd, int departure_time, bool prior, bool preset)
    :finish(false),wait(false),prior(prior),preset(preset),id(id),s_id(s_id),e_id(e_id),max_spd(max_spd),
    departure_time(departure_time),spend_time(0),
    wait_len(0),priority(3),road_index(-1),lane(-1),place(-1),finish_time(0),path_ptr(0){
    };
    void setWait(bool b){
        wait=b;
        if(!b)wait_other_road = false;
    };
    void setWaitOtherRoad(bool b){wait_other_road = b;};
    bool getWaitOtherRoad(){return wait_other_road;};
    bool getWait(){return wait;};
    void setWaitLen(int t){
        wait = true;
        wait_len = t;
    };
    int getWaitLen(){
        return wait_len;
    };
    int getID(){return id;};
    int getMax_spd(){return max_spd;};
    int getS_id(){return s_id;};
    int getE_id(){return e_id;};
    void setFinish(int t){
        finish=true;
        finish_time = t;
    };
    bool getFinish(){return finish;};
    int getFinishTime(){return finish_time;}
    int getDepartureTime(){return departure_time;};
    /* int getNextRoadID(){
        if(!p.empty())return p.front();
        else return -1;
    }; */
    int getNextRoadID(){
        if(path_ptr<road_path.size())return road_path[path_ptr];
        return -1;
    }
    int getPriority(){return priority;}
    bool getPrior(){return prior;}
    bool getPreset(){return preset;}
    void setPriority(int p){priority = p;};
    void popRoad(){if(!p.empty())p.pop();}
    void pushRoad(int road_id){p.push(road_id);};
    void setPath(queue<int>&& path){p = path;};
    void setPlace(int r,int l, int p){
        this->road_index=r;
        this->lane=l;
        this->place=p;};
    void printPlace(){cout<<id<<" "<<road_index<<" "<<lane<<" "<<place<<endl;};
    int getConsumeTime(){return finish_time-departure_time;};
    void setPlanTime(int t){
        plan_time = max(t,departure_time);
    };
    int getPlanTime(){return plan_time;};
    void setDirection(bool u,bool r){
        up = u;
        right = r;
    }
    int getDirection(){
        return up*2+right;
    }
    void setBestCondition(){
        best_plan_time = plan_time;
        best_path = road_path;
    }
    int getBestPlanTime(){
        return best_plan_time;
    }
    vector<int>& getBestPath(){
        return best_path;
    }
    vector<int> road_path;
    void init(){
        while(!p.empty())p.pop();
        if(!preset){
            //if(id==67785)cout<<"67785 path clear!!!!!!!!!"<<endl;
            road_path.clear();
            best_path.clear();
            best_plan_time = 0;
        }
        else{
            for(const auto path : road_path){
                p.push(path);
            }
        }
        path_ptr = 0;
        wait = false;
        wait_other_road = false;
        finish = false;
        road_set.clear();
    }
    void setTempTime(){
        temp_time = best_plan_time;
    }
    int temp_time;
    void setSID(int sid){
        s_id = sid;
    }
    void setEID(int eid){
        e_id = eid;
    }
    void setOnRoadTime(int t){
        on_road_time = t;
    }
    int getOnRoadTime(){
        return on_road_time;
    }
    int getDistance(){
        return distance;
    }
    void setDistance(int d){
        distance = d;
    }
    int getPathPtr(){
        return path_ptr;
    }
    void MovePathPtr(){
        road_set.insert(road_path[path_ptr]);
        /* if(id==100799){
            cout<<"MOVE "<<path_ptr<<endl;
        } */
        path_ptr++;
    }
    bool canGo(int road_index){
        return (road_set.find(road_index)==road_set.end());
    }
    set<int> road_set;
    //int chooseDirection();
private:
    bool up;
    bool right;
    bool finish;
    bool wait;
    bool wait_other_road;
    const bool prior;
    const bool preset;
    int id;
    int s_id;
    int e_id;
    int max_spd;
    int departure_time;
    int plan_time;
    int best_plan_time;
    int spend_time;
    int wait_len;//等待前已经行驶距离
    int priority;//直行:0,左转:1,右转:2,default:3
    int road_index;
    int lane;
    int place;
    int finish_time;
    int on_road_time;
    int distance;
    size_t path_ptr;
    queue<int> p;//p.front()为下一条道路ID,p可能为空
    vector<int> best_path;
     
};

#endif