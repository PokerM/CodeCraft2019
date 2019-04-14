#include "Scheduler.h"
#include <algorithm>
#include "TxtHandler.h"
#include <cmath>

using namespace std;

struct CmpAnswerByStartTime_v2{
    bool operator()(Answer& lhs, Answer& rhs){
        int l = lhs.getStartTime();
        int r = rhs.getStartTime();
        return l < r;
    }
}cbst_v2;

struct CmpByCarID{
    bool operator()(int lhs, int rhs){
        return car_ve[lhs].getID()<car_ve[rhs].getID();
    }
}cbid;

struct CmpByCrossID{
    bool operator()(int lhs, int rhs){
        return cross_ve[lhs].getID()<cross_ve[rhs].getID();
    }
}cb_cross_id;

void Scheduler::scheduleCarsOnRoad(int road_index){
    Road& r = road_ve[road_index];
    scheduleCarsOnRoad(road_index,r.car_on_road,false);
    if(r.getIs_two_way())scheduleCarsOnRoad(road_index,r.car_on_road_r,true);
}

double reserveDecimal(double in,int num){
    int e = pow(10,num);
    in = in * e;
    int in_int = static_cast<int>(in);
    return static_cast<double>(in_int)/e;
}

Scheduler::Scheduler():car_finish(0),dead_lock_union(2*road_ve.size(),-1){
    for(size_t i=0;i<cross_ve.size();i++)cross_id_inc.push_back(i);
    sort(cross_id_inc.begin(),cross_id_inc.end(),cb_cross_id);
};
/* 
调度所有在指定道路上的车辆
 */
void Scheduler::scheduleCarsOnRoad(const int road_index,vector<vector<pair<int,int> > >& car_on_road, bool reverse){
    int end_cross;//当前道路出口
    if(!reverse)end_cross = road_ve[road_index].getE_id();
    else end_cross = road_ve[road_index].getS_id();
    for(size_t i=0;i<car_on_road.size();i++){
        for(size_t j=0;j<car_on_road[i].size();j++){
            int car_index = (car_on_road[i][j]).first;
            int place = (car_on_road[i][j]).second;
            int spd = car_ve[car_index].getMax_spd()<road_ve[road_index].getLimit_spd()?
                        car_ve[car_index].getMax_spd():road_ve[road_index].getLimit_spd();
            int len = road_ve[road_index].getLen();
            Car* car = &car_ve[car_index];
            if(j==0){                                               //每条道路线上的第一辆车
                if(spd+place>len){                                  //判断是否行驶出路口
                    all_car_end = false;
                    car->setWait(true);//设置等待前已经行驶距离
                    car->setWaitLen(len-place);
                    int priority = getPriority(road_index,end_cross,car);
                    car->setPriority(priority);
                }
                else{
                    (car_on_road[i][j]).second += spd;
                    car->setPlace(road_index,i,(car_on_road[i][j]).second);
                    car->setWait(false);
                }
            }
            else{                                                   //根据前车判断车辆状态
                auto car_before_index = (car_on_road[i][j-1]).first;
                auto place_before = (car_on_road[i][j-1]).second;
                Car* car_before = &car_ve[car_before_index];
                //cout<<"before"<<place_before<<endl;
                if(spd+place>=place_before){
                    if(car_before->getWait()){//判断前车状态
                        all_car_end = false;
                        car->setWait(true);
                    }
                    else{
                        (car_on_road[i][j]).second = place_before -1;
                        car->setPlace(road_index,i,(car_on_road[i][j]).second);
                        car->setWait(false);
                    }
                }
                else{
                    (car_on_road[i][j]).second += spd;
                    car->setPlace(road_index,i,(car_on_road[i][j]).second);
                    car->setWait(false);
                }
            }
        }
    }
}

/* 
调度等待出路口车辆
*/
void Scheduler::scheduleCarsOnCross(int cross_index){
    const int* cur_road_id = cross_ve[cross_index].getRoadID();
    int inc_index[4]={0,1,2,3};
    for(size_t i=0;i<4;i++){
        for(size_t j=i+1;j<4;j++){
            Road& l_road = road_ve[cur_road_id[inc_index[i]]];
            Road& r_road = road_ve[cur_road_id[inc_index[j]]];
            if(l_road.getID()>r_road.getID()){
                int t = inc_index[i];
                inc_index[i] = inc_index[j];
                inc_index[j] = t;
            }
        }
    }
    for(size_t k=0;k<4;k++){
        if(cur_road_id[inc_index[k]]>=0){
            int road_index = cur_road_id[inc_index[k]];
            Road& r = road_ve[road_index];
            if(r.getS_id()==cross_index){
                if(r.getIs_two_way()){
                    scheduleWaitCars(cross_index,road_index,r.car_on_road_r,true);
                }
            }else if(r.getE_id()==cross_index){
                scheduleWaitCars(cross_index,road_index,r.car_on_road,false);
            }
            else{
                cerr<<"scheduleCarsOnCross:road "<<road_index<<" do not link to cross "<<cross_index<<endl;
            }
        }
        //cout<<cross_index<<" "<<over[0]<<" "<<over[1]<<" "<<over[2]<<" "<<over[3]<<" "<<endl;
    }
}

/* 
调度指定道路上等待车辆
首先得到最高优先级车辆,调度该车辆,若调度成功,则重复调度,直到所有车辆处于终止状态,返回true
若需要等待下一道路车辆,则使得all_car_end为false,并返回true,不再调度
若有冲突,返回false
 */
bool Scheduler::scheduleWaitCars(int cross_index, size_t road_index, vector<vector<pair<int,int> > >& car_on_road,bool reverse){
    if(road_index>=road_ve.size())cerr<<"scheduleWaitCars road_index="<<road_index<<endl;
    Road& r = road_ve[road_index];

    while(1){
        /* 
        第一步:得到最高优先级车辆
        */
        int max_ = getMostPriorityLane(car_on_road);
        if(max_>=0){
            int i = max_;
            int car_index = car_on_road[i][0].first;
            Car& car = car_ve[car_index];
            if(car.getWait()){                  //是否处于等待状态
                //if(place != r.getLen())cerr<<"scheduleWaitCars:wrong status"<<endl;
                int priority = car.getPriority();
                bool prior = car.getPrior();
                int can_go = compare_priority(cross_index,road_index,priority,reverse,prior);
                if(can_go==-1){
                    car.setWaitOtherRoad(true);
                    all_car_end = false;
                    int next_road = car.getNextRoadID();
                    if(next_road==-1){//处理到达终点的情况
                        const int* cur_cross_road_index = cross_ve[cross_index].getRoadID();
                        int cur_r = 0;
                        for(size_t k=0;k<4;k++){
                            if(road_index==(size_t)cur_cross_road_index[k]){
                                cur_r = k;
                                break;
                            }
                        }
                        next_road = cur_r+2<4?cur_cross_road_index[cur_r+2]:cur_cross_road_index[cur_r-2];
                    }
                    bool next_road_reverse  = true;
                    if(cross_index==road_ve[next_road].getS_id()){
                        next_road_reverse = false;
                    }
                    else if(cross_index==road_ve[next_road].getE_id()){
                        next_road_reverse = true;
                    }
                    else{
                        if(next_road!=-1){
                            cerr<<"scheduleWaitCars:road "<<road_ve[next_road].getID()<<"do not link to cross "<<cross_ve[cross_index].getID()<<endl;
                            throw next_road;
                        }
                    }
                    if(next_road!=-1){ 
                        int next_road_union_id = next_road_reverse?next_road+road_ve.size():next_road;
                        int road_union_id = reverse?road_index+road_ve.size():road_index;
                        int t = next_road_union_id;
                        if(dead_lock_union[road_union_id]==-1){//死锁检测
                            while(dead_lock_union[t]!=-1){
                                t = dead_lock_union[t];
                            }
                            if(t == road_union_id){
                                cerr<<"cur time "<<cur_time<<endl;
                                cerr<<"cross "<<cross_ve[cross_index].getID()<<" road"<<road_union_id<<" next road"<<next_road_union_id<<" dead lock!"<<endl;
                                printDeadLockUnion();
                                throw cross_index;
                            }
                            dead_lock_union[road_union_id] = t;
                        }
                    }
                    return true;
                }
                if(can_go==0){
                    all_car_end = false;
                    return true;
                }
                if(can_go==1){
                    if(car.getE_id()==cross_index){ //到达终点
                        car.setFinish(cur_time);
                        car.setWait(false);
                        car_finish++;
                        if(!car_on_road[i].empty())car_on_road[i].erase(car_on_road[i].begin());
                        scheduleCarOnLane(cross_index,road_index,car_on_road[i]);
                    }
                    else{
                        int wait_spd = car.getWaitLen();
                        int next_road = car.getNextRoadID();
                        int car_spd = car.getMax_spd();
                        int n_road_spd = road_ve[next_road].getLimit_spd();
                        Road& n_road = road_ve[next_road];
                        int spd = car_spd<n_road_spd?car_spd:n_road_spd;
                        if(spd>wait_spd){
                            /* 
                            让该车行驶入下一条车道
                            */
                            int res = 1;
                            bool next_road_reverse = true;
                            if(cross_index==road_ve[next_road].getS_id()){
                                next_road_reverse = false;
                                res = toRoad(car_index,next_road,n_road.car_on_road,false);
                            }
                            else if(cross_index==road_ve[next_road].getE_id()){
                                next_road_reverse = true;
                                res = toRoad(car_index,next_road,n_road.car_on_road_r,true);
                            }    
                            else{
                                cerr<<"car ID:"<<car.getID()<<" Last road:"<<road_ve[road_index].getID()<<endl;
                                cerr<<"scheduleWaitCars:road "<<road_ve[next_road].getID()<<"do not link to cross "<<cross_ve[cross_index].getID()<<endl;
                                throw next_road;
                            }
                            if(res==0){
                                if(!car_on_road[i].empty())car_on_road[i].erase(car_on_road[i].begin());
                                //r.deleteCar(reverse,car.getMax_spd());
                                //if(test)r.deleteCar(car_index,reverse,car.getPreset());
                                car.setWait(false);
                                scheduleCarOnLane(cross_index,road_index,car_on_road[i]);
                            }
                            else if(res==-1){
                                //cout<<"scheduleWaitCars:无法进入下一条道路 car_id="<<car_index<<endl;
                                car_on_road[i][0].second = r.getLen();
                                car.setWait(false);
                                scheduleCarOnLane(cross_index,road_index,car_on_road[i]);
                            }
                            else if(res==1){
                                car.setWaitOtherRoad(true);
                                int next_road_union_id = next_road_reverse?next_road+road_ve.size():next_road;
                                int road_union_id = reverse?road_index+road_ve.size():road_index;
                                int t = next_road_union_id;
                                if(dead_lock_union[road_union_id]==-1){//死锁检测
                                    while(dead_lock_union[t]!=-1){
                                        t = dead_lock_union[t];
                                    }
                                    if(t == road_union_id){
                                        cerr<<"cur time "<<cur_time<<endl;
                                        cerr<<"cross "<<cross_ve[cross_index].getID()<<" road"<<road_union_id<<" next road"<<next_road_union_id<<" dead lock!"<<endl;
                                        printDeadLockUnion();
                                        throw cross_index;
                                    }
                                    dead_lock_union[road_union_id] = t;
                                }
                                all_car_end = false;
                                return true;
                            }
                        }
                        else{//无法驶入下一条道路
                            car_on_road[i][0].second = r.getLen();
                            car.setWait(false);
                            scheduleCarOnLane(cross_index,road_index,car_on_road[i]);
                        }
                    }
                }
            }
        }
        else
            return true;
    }
}

/* 
车道上第一辆车进入下一路口或者到达终点时,调度指定车道上的wait car
 */
void Scheduler::scheduleCarOnLane(int cross_index,int road_index,vector<pair<int,int> > & car_queue){
    Car* car;
    Road& r = road_ve[road_index];
    for(size_t i=0;i<car_queue.size();i++){
        int car_index = car_queue[i].first;
        car = &(car_ve[car_index]);
        if(car->getWait()){
            //int wait_spd = car->getWaitLen();//已经行驶距离
            int spd = car->getMax_spd()<r.getLimit_spd()?car->getMax_spd():r.getLimit_spd();
            if(spd<=0)cerr<<"scheduleCarOnLane:spd<0"<<endl;
            int place = car_queue[i].second;
            if(i==0){
                int len = r.getLen();
                if(spd+place>len){//判断是否行驶出路口
                    all_car_end = false;
                    car->setWait(true);//设置等待前已经行驶距离
                    car->setWaitLen(len-place);
                    int priority = getPriority(road_index,cross_index,car);
                    car->setPriority(priority);
                }
                else{
                    (car_queue[i]).second += spd;
                    car->setPlace(road_index,i,(car_queue[i]).second);
                    car->setWait(false);
                }
            }
            else{                                                   //根据前车判断车辆状态
                auto car_before_index = (car_queue[i-1]).first;
                auto place_before = (car_queue[i-1]).second;
                Car* car_before = &car_ve[car_before_index];
                //cout<<"before"<<place_before<<endl;
                if(spd+place>=place_before){
                    if(car_before->getWait()){//判断前车状态
                        car->setWait(true);
                        all_car_end = false;
                    }
                    else{
                        (car_queue[i]).second = place_before -1;
                        car->setPlace(road_index,i,(car_queue[i]).second);
                        car->setWait(false);
                    }
                }
                else{
                    (car_queue[i]).second += spd;
                    car->setPlace(road_index,i,(car_queue[i]).second);
                    car->setWait(false);
                }
            }
        }
    }
/*     int last_car_index = car_queue.back().first;
    if(!car_ve[last_car_index].getWait()){ */
    bool reverse = true;
    if(road_ve[road_index].getE_id()==cross_index)reverse = false;
    scheduleCarInGarage(pri_wait_cars,road_index,reverse);
    //scheduleCarInGarage(pri_wait_cars);
    //}
}

/* 
得到等待车辆的优先级
 */
int Scheduler::getPriority(int road_index,int cross_index,Car* car){
    Cross& c = cross_ve[cross_index];
    const int* cur_road_id = c.getRoadID();
    int next_road = car->getNextRoadID();
    if(next_road==-1)return 0;
    int c_num,n_num;
    for(size_t i=0;i<4;i++){
        if(cur_road_id[i]==road_index)c_num=i;
        if(cur_road_id[i]==next_road)n_num=i;
        //cout<<cur_road_id[i]<<" ";
    }
    //cout<<endl;
    int t = c_num - n_num;
    if(t==2||t==-2)return 0;//直行
    if(t==-1||t==3)return 1;//左转
    if(t==1||t==-3)return 2;//右转
    //for(int i=0;i<4;i++)cout<<cur_road_id[i]<<" ";
    //cout<<endl;
    //cerr<<"getPriority:wrong priority! road id = "<<road_index<<" "<<next_road<<" cross id = "<<cross_index<<endl;
    cerr<<"cross: "<<cross_ve[cross_index].getID()<<endl;
    cerr<<(cur_road_id[0]==-1?-1:road_ve[cur_road_id[0]].getID())<<" "
        <<(cur_road_id[1]==-1?-1:road_ve[cur_road_id[1]].getID())<<" "
        <<(cur_road_id[2]==-1?-1:road_ve[cur_road_id[2]].getID())<<" "
        <<(cur_road_id[3]==-1?-1:road_ve[cur_road_id[3]].getID())<<endl;
    cerr<<"cur road "<<road_ve[road_index].getID()<<" next road "<<road_ve[next_road].getID()<<endl;
    cerr<<"car id "<<car->getID()<<endl;
    throw -1;
    return -1;
}

/* 
1:没有冲突,可以调度
0:冲突,停止调度
-1:等待
 */
int Scheduler::compare_priority(int cross_index,int road_index,int priority,bool reverse,bool prior){
    if(prior&&(priority==0))return 1;
    Cross& c = cross_ve[cross_index];
    const int* cur_road_id = c.getRoadID();
    int r=-1;
    for(size_t i=0;i<4;i++){
        if(cur_road_id[i]==road_index){
            r = i;
            break;
        }
    }
    if(r==-1){
        cout<<"compare_priority:r==-1"<<endl;
        throw -1;
    }
    if(priority==0){//非优先车辆直行
        int r_index = r-1>=0?r-1:3;
        int l_index = r+1<4?r+1:0;
        int r_road_id = cur_road_id[r_index];
        int l_road_id = cur_road_id[l_index];
        int r_s = 1, l_s = 1;
        if(r_road_id>=0){
            Road& r = road_ve[r_road_id];
            if(r.getE_id()==cross_index){
                r_s = collisionDetection(r.car_on_road,2,true);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                r_s = collisionDetection(r.car_on_road_r,2,true);
            }
        }
        if(l_road_id>=0){
            Road& r = road_ve[l_road_id];
            if(r.getE_id()==cross_index){
                l_s = collisionDetection(r.car_on_road,1,true);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                l_s = collisionDetection(r.car_on_road_r,1,true);
            }
        }
        if(r_s==1&&l_s==1)return 1;
        if(r_s==0||l_s==0)return 0;
        return -1;
    }
    if(priority==1){//左转
        int s = r-1>=0?r-1:3;
        int r_index = r-2>=0?r-2:r+2; 
        int s_road_id = cur_road_id[s];
        int r_road_id = cur_road_id[r_index];
        if(prior){
            if(s_road_id>=0){
                Road& r = road_ve[s_road_id];
                if(r.getE_id()==cross_index){
                    return collisionDetection(r.car_on_road,0,true);
                }
                else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                    return collisionDetection(r.car_on_road_r,0,true);
                }
                else{//该道路为单行道，且其出口不为该道路
                    return 1;
                }     
            }
            else{
                return 1;
            }
        }
        else{//非优先车辆的左转判断
            int s_s = 1, r_s = 1;
            if(s_road_id>=0){
                Road& r = road_ve[s_road_id];
                if(r.getE_id()==cross_index){
                    s_s = collisionDetection(r.car_on_road,0,false);
                }
                else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                    s_s = collisionDetection(r.car_on_road_r,0,false);
                }
            }
            if(r_road_id>=0){
                Road& r = road_ve[r_road_id];
                if(r.getE_id()==cross_index){
                    r_s = collisionDetection(r.car_on_road,2,true);
                }
                else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                    r_s = collisionDetection(r.car_on_road_r,2,true);
                }
            }
            if(s_s==1&&r_s==1)return 1;
            if(s_s==0||r_s==0)return 0;
            return -1;  
        }
    }
    if(priority==2){//右转
        int b_s = 1,b_l=1;
        /* 
        直行检测
         */
        int s = r + 1 <4?r+1:0;
        int s_road_id = cur_road_id[s];
        if(s_road_id>=0){
            Road& r = road_ve[s_road_id];
            if(r.getE_id()==cross_index){
                b_s = collisionDetection(r.car_on_road,0,prior);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                b_s = collisionDetection(r.car_on_road_r,0,prior);
            }
            else{
                b_s = 1;
            }
        }
        else{
            b_s = 1;
        }
        /* 
        左转检测
         */
        int l = r + 2 < 4? r+2:r-2;
        int l_road_id = cur_road_id[l];
        if(l_road_id>=0){
            Road& r = road_ve[l_road_id];
            if(r.getE_id()==cross_index){
                b_l = collisionDetection(r.car_on_road,1,prior);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                b_l = collisionDetection(r.car_on_road_r,1,prior);
            }
            else{//
                b_l = 1;
            }
        }
        else{//道路编号小于0,没有道路
            b_l = 1;
        }
        if(b_l==1&&b_s==1)return 1;
        if(b_l==0||b_s==0)return 0;
        return -1;
    }
    cerr<<"wrong priority "<<priority<<endl;
    throw priority;
    return 0;
}

/* 
1:没有冲突,可以调度
0:冲突,停止调度
-1:等待
 */
int Scheduler::compare_priority(int cross_index,int road_index,int priority,bool reverse){
    if(priority==0)return 1;
    Cross& c = cross_ve[cross_index];
    const int* cur_road_id = c.getRoadID();
    int r=-1;
    for(size_t i=0;i<4;i++){
        if(cur_road_id[i]==road_index){
            r = i;
            break;
        }
    }
    if(r==-1){
        cout<<"compare_priority:r==-1"<<endl;
        return false;
    }

    if(priority==1){//左转
        int s = r-1>=0?r-1:3;
        if(s>=0&&s<4){
            int s_road_id = cur_road_id[s];
            if(s_road_id>=0){
                Road& r = road_ve[s_road_id];
                if(r.getE_id()==cross_index){
                    return collisionDetection(r.car_on_road,0);
                }
                else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                    return collisionDetection(r.car_on_road_r,0);
                }
                else{//该道路为单行道，且其出口不为该道路
                    return 1;
                }     
            }
            else{
                return 1;
            }
        }
    }
    if(priority==2){//右转
        int b_s = 1,b_l=1;
        /* 
        直行检测
         */
        int s = r + 1 <4?r+1:0;
        int s_road_id = cur_road_id[s];
        if(s_road_id>=0){
            Road& r = road_ve[s_road_id];
            if(r.getE_id()==cross_index){
                b_s = collisionDetection(r.car_on_road,0);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                b_s = collisionDetection(r.car_on_road_r,0);
            }
            else{
                b_s = 1;
            }
        }
        else{
            b_s = 1;
        }
        /* 
        左转检测
         */
        int l = r + 2 < 4? r+2:r-2;
        int l_road_id = cur_road_id[l];
        if(l_road_id>=0){
            Road& r = road_ve[l_road_id];
            if(r.getE_id()==cross_index){
                b_l = collisionDetection(r.car_on_road,1);
            }
            else if(r.getIs_two_way()&&r.getS_id()==cross_index){
                b_l = collisionDetection(r.car_on_road_r,1);
            }
            else{//
                b_l = 1;
            }
        }
        else{//道路编号小于0,没有道路
            b_l = 1;
        }
        if(b_l==1&&b_s==1)return 1;
        if(b_l==0||b_s==0)return 0;
        return -1;
    }
    cerr<<"wrong priority "<<priority<<endl;
    throw priority;
    return 0;
}
/* 
1:没有冲突,可以调度
0:冲突,停止调度
-1:等待
 */
int Scheduler::collisionDetection(vector<vector<pair<int,int> > >& car_on_road,int priority){
    if(car_on_road.empty())return 1;
    int max_ = getMostPriorityLane(car_on_road);
    if(max_>0){
        int i = max_;
        int car_index = car_on_road[i][0].first;
        Car& car = car_ve[car_index];
        if(car.getWait()){
            if(car.getPriority()==priority){
                if(car.getWaitOtherRoad()){
                    return -1;
                }
                return 0;
            }
            return 1;
        }
    }
    return 1;
}

/* 
让车辆驶入道路
车辆分两种
一:从车库中出来
二:从其他道路过来
1:等待其他车辆
0:上路成功
-1:上路失败
 */
int Scheduler::toRoad(int car_index, int road_index, vector<vector<pair<int,int> > >& car_on_road,bool reverse){
    int i=0;
    Car& car = car_ve[car_index];
    Road& r = road_ve[road_index];
    int spd;
    bool other_road = true;
    if(car.getWait()){//从其他道路过来
        spd = min(r.getLimit_spd(),car.getMax_spd());
        spd = spd - car.getWaitLen();
        if(spd<=0)cerr<<"toRoad:spd<0"<<endl;
        other_road = true;
    }
    else{//刚从车库出来
        other_road = false;
        spd = car.getMax_spd()<r.getLimit_spd()?car.getMax_spd():r.getLimit_spd();
    } 
    while(i<r.getLane_num()){
        //if(print)
            //cout<<"car_on_road size "<<car_on_road[i].size()<<endl;
        if(!car_on_road[i].empty()){//当前道路不为空
            //if(print)cout<<"here"<<endl;
            int car_f_index = (car_on_road[i].back()).first;
            Car& car_f = car_ve[car_f_index];
            int place = (car_on_road[i].back()).second;
            if(spd>=place){
                if(car_f.getWait()){
                    if(other_road)return 1;
                    return -1;
                }
                if(place>1){
                    (car_on_road[i]).push_back({car_index,place-1});
                    car.setPlace(road_index,i,place-1);
                    car.setWait(false);
                    //car.popRoad();
                    car.MovePathPtr();
                    //if(car.getID()==22159)cout<<"MOVE1"<<endl;
                    //if(test)r.upDateFactor(true,reverse);
                    return 0;
                }
            }
            else{
                (car_on_road[i]).push_back({car_index,spd});
                car.setPlace(road_index,i,spd);
                car.setWait(false);
                //car.popRoad();
                car.MovePathPtr();
                //if(car.getID()==22159)cout<<"MOVE2"<<endl;
                //if(test)r.upDateFactor(true,reverse);
                return 0;
            }
        }
        else{//当前道路为空
             if(spd>r.getLen()){//车辆行驶距离大于车道长度,此处假定车道长度大于车辆速度
                cerr<<"car spd > road len"<<endl;
                return -1;
             }
             else{
                //if(print)
                    //cout<<"road empty"<<endl;
                car_on_road[i].push_back({car_index,spd});
                //car.setPlace(road_index,i,spd);
                car.setWait(false);
                //car.popRoad();
                car.MovePathPtr();
                return 0;
             }
        }
        i++;
    }
    //上道失败
    return -1;
}

void Scheduler::scheduleCars(vector<int>& start_cars, vector<int>& prior_start_cars, int ct){
    cur_time = ct;
    for(const auto psc : prior_start_cars){
        pri_wait_cars.push_back(psc);
    }
    for(const auto sc : start_cars){
        wait_cars.push_back(sc);
    }
    /* 
    第一步:调度所有道路上的车辆
     */
    for(size_t i=0;i<road_ve.size();i++){
        scheduleCarsOnRoad(i);
    }
    /* 
    第二步:调度所有等待车辆
     */
    all_car_end = false;
    int time_ = 0;
    scheduleCarInGarage(pri_wait_cars);
    //for(const auto u : dead_lock_union)if(u!=-1)throw "u!=-1";
    while(!all_car_end){
        all_car_end = true;
        vector<int> t(2*road_ve.size(),-1);
        dead_lock_union.swap(t);
        //scheduleCarInGarage(pri_wait_cars);
        for(size_t i=0;i<cross_id_inc.size();i++){
            scheduleCarsOnCross(cross_id_inc[i]);
        }
        time_++;
        if(time_%100==0)
            cout<<"time="<<time_<<endl;
        if(time_>1000){
            TxtHandler th;
            th.logRoadStatus(cur_time);
            for(size_t i=0;i<road_ve.size();i++){
                Road& r = road_ve[i];
                for(size_t j=0;j<r.car_on_road.size();j++){
                    if(!r.car_on_road[j].empty()){
                        int car_index = r.car_on_road[j][0].first;
                        if(car_ve[car_index].getWait()){
                            int sid = r.getS_id();
                            int eid = r.getE_id();
                            int next_road = car_ve[car_index].getNextRoadID();
                            Road& nr = road_ve[next_road];
                            int nsid = nr.getS_id();
                            int neid = nr.getE_id();
                            int cross;
                            if(sid==nsid||sid==neid)cross = sid;
                            else cross = eid;
                            int reverse = cross==nsid?0:1;
                            cerr<<"cross "<<cross_ve[cross].getID()<<" road "<<r.getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()
                                 <<" next road "<<road_ve[car_ve[car_index].getNextRoadID()].getID()<<" reverse "
                                 <<to_string(reverse)<<" car spd "<<car_ve[car_index].getMax_spd()
                                 <<" priorty "<<car_ve[car_index].getPriority()<<" prior "<<car_ve[car_index].getPrior()<<" still wait"<<endl;
                        }
                    }
                }
                if(r.getIs_two_way()){
                    for(size_t j=0;j<r.car_on_road_r.size();j++){
                        if(!r.car_on_road_r[j].empty()){
                            int car_index = r.car_on_road_r[j][0].first;
                            if(car_ve[car_index].getWait()){
                                int sid = r.getS_id();
                                int eid = r.getE_id();
                                int next_road = car_ve[car_index].getNextRoadID();
                                Road& nr = road_ve[next_road];
                                int nsid = nr.getS_id();
                                int neid = nr.getE_id();
                                int cross;
                                if(sid==nsid||sid==neid)cross = sid;
                                else cross = eid;
                                int reverse = cross==nsid?0:1;
                                cerr<<"cross "<<cross_ve[cross].getID()<<" reverse road "<<r.getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()
                                    <<" next road "<<road_ve[car_ve[car_index].getNextRoadID()].getID()<<" reverse "
                                    <<to_string(reverse)<<" car spd "<<car_ve[car_index].getMax_spd()
                                    <<" priorty "<<car_ve[car_index].getPriority()<<" prior "<<car_ve[car_index].getPrior()<<" still wait"<<endl;
                            }
                        }
                    }
                }
            }
            cerr<<"cur_time "<<cur_time<<endl;
            throw time_;
        }
        dead_lock_union.clear();
    }
    /* 
    test 
     */
    for(size_t i=0;i<road_ve.size();i++){
        Road& r = road_ve[i];
        for(size_t j=0;j<r.car_on_road.size();j++){
            if(!r.car_on_road[j].empty()){
                int car_index = r.car_on_road[j][0].first;
                if(car_ve[car_index].getWait()){
                    cerr<<"cur time "<<cur_time<<" road "<<road_ve[i].getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()<<" still wait"<<endl;
                }
            }
        }
        if(r.getIs_two_way()){
            for(size_t j=0;j<r.car_on_road_r.size();j++){
                if(!r.car_on_road_r[j].empty()){
                    int car_index = r.car_on_road_r[j][0].first;
                    if(car_ve[car_index].getWait()){
                        cerr<<"cur time "<<cur_time<<" reverse road "<<road_ve[i].getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()<<" still wait"<<endl;
                    }
                }
            }
        }
    }
    scheduleCarInGarage(pri_wait_cars);
    scheduleCarInGarage(wait_cars);
    //cout<<"pri_wait_cars size "<<pri_wait_cars.size()<<" wait cars size "<<wait_cars.size;
}

/* 
调度等待上路的车辆
 */
void Scheduler::scheduleCarInGarage(vector<int>& start_cars){
    vector<int> temp;
    for(size_t i=0;i<start_cars.size();i++){
        Car& car = car_ve[start_cars[i]];
        int cross = car.getS_id();
        int road_index = car.getNextRoadID();
        int res = -1;
        Road& road = road_ve[road_index];
        if(road_ve[road_index].getS_id()==cross){
            res = toRoad(start_cars[i],road_index,road.car_on_road,false);
        }
        else if(road_ve[road_index].getE_id()==cross){
            res = toRoad(start_cars[i],road_index,road.car_on_road_r,true);
        }
        if(res != 0){
            temp.push_back(start_cars[i]);
        }
        else{
            car_ve[start_cars[i]].setOnRoadTime(cur_time);
        }
    }
    start_cars.swap(temp);
    temp.clear();
}
/* 
得到当前道路的最高优先级车辆
 */
int Scheduler::getMostPriorityLane(vector<vector<pair<int,int> > >& car_on_road){
    int max_ = -1, max_place = -1;
    bool priority = false;
    for(size_t i=0;i<car_on_road.size();i++){
        if(!car_on_road[i].empty()){
            int car_index = car_on_road[i][0].first;
            int place = car_on_road[i][0].second;
            Car& car = car_ve[car_index];
            if(car.getWait()){
                bool cur_priority = car.getPrior();
                if(priority){
                    if(cur_priority&&max_place<place){
                        max_ = i;
                        max_place = place;
                    }
                }
                else{
                    if(cur_priority){
                        max_ = i;
                        max_place = place;
                        priority = true;
                    }
                    else if(max_place<place){
                        max_ = i;
                        max_place = place;
                    }
                }
            }
        }
    }
    return max_;
}

int Scheduler::collisionDetection(vector<vector<pair<int,int> > >& car_on_road,int priority,bool prior){
    if(car_on_road.empty())return 1;
    int max_ = getMostPriorityLane(car_on_road);
    if(max_>=0){
        int i = max_;
        int car_index = car_on_road[i][0].first;
        Car& car = car_ve[car_index];
        if(car.getWait()){
            if(car.getPriority()==priority){
                if(!prior||car.getPrior()){
                    if(car.getWaitOtherRoad()){
                        return -1;
                    }
                    return 0;
                }
            }
            return 1;
        }
    }
    return 1;
}

/* 
调度所有车辆
 */
int Scheduler::scheduleAllCars(vector<Answer>& answer_ve){
    int time_consume = 0;
    car_finish = 0;
    size_t ans_ptr = 0;
    sort(answer_ve.begin(),answer_ve.end(),cbst_v2);
    while(time_consume<10000&&car_finish<answer_ve.size()){
        vector<int> prior_start_cars,start_cars;
        while(ans_ptr<answer_ve.size()&&time_consume+1==answer_ve[ans_ptr].getStartTime()){
            int car_index = answer_ve[ans_ptr].getCarID();
            Car& car = car_ve[car_index];
			car.setPlanTime(answer_ve[ans_ptr].getStartTime());
			//for(const auto p : answer_ve[ans_ptr].path)car.pushRoad(p);
			car.road_path.assign(answer_ve[ans_ptr].path.begin(),answer_ve[ans_ptr].path.end());
            car.setBestCondition();
            if(car.getPrior())prior_start_cars.push_back(car_index);
            else start_cars.push_back(car_index);
            ans_ptr++;
        }
        //cout<<"start time = "<<time_consume+1;
        sort(start_cars.begin(),start_cars.end(),cbid);
        sort(prior_start_cars.begin(),prior_start_cars.end(),cbid);
        /* for(size_t i=0;i<start_cars.size();i++)cout<<car_ve[start_cars[i]].getID()<<" ";
        cout<<endl; */
        scheduleCars(start_cars,prior_start_cars,time_consume+1);
        start_cars.clear();
        prior_start_cars.clear();
        time_consume++;
    }
    int total_car_time = 0;
    int prior_car_time = 0;
    int prior_car_start_time = INT32_MAX, prior_car_end_time = 0;
    int max_prior_car_start_time = 0;
    int min_car_start_time = INT32_MAX, max_car_start_time = 0;
    int prior_car_num = 0;
    int max_speed = 0,min_speed = INT32_MAX,max_prior_speed = 0,min_prior_speed = INT32_MAX;
    set<int> start,end,prior_start,prior_end;
    for(size_t i=0;i<car_ve.size();i++){
        Car& car = car_ve[i];
        //if(car.getConsumeTime()<0)throw -1;
        total_car_time += car.getConsumeTime();
        max_speed = max_speed>car.getMax_spd()?max_speed:car.getMax_spd();
        min_speed = min_speed<car.getMax_spd()?min_speed:car.getMax_spd();
        min_car_start_time = min_car_start_time<car.getDepartureTime()?min_car_start_time:car.getDepartureTime();
        max_car_start_time = max_car_start_time>car.getDepartureTime()?max_car_start_time:car.getDepartureTime();
        start.insert(car.getS_id());
        end.insert(car.getE_id());
        if(car.getPrior()){
            prior_car_num++;
            max_prior_speed = max(max_prior_speed,car.getMax_spd());
            min_prior_speed = min(min_prior_speed,car.getMax_spd());
            prior_start.insert(car.getS_id());
            prior_end.insert(car.getE_id());
            prior_car_time += car.getConsumeTime();
            max_prior_car_start_time = max(max_prior_car_start_time,car.getDepartureTime());
            if(car.getDepartureTime()<prior_car_start_time)prior_car_start_time = car.getDepartureTime();
            if(car.getFinishTime()>prior_car_end_time)prior_car_end_time = car.getFinishTime();
        }
    }
    if(debug){
        TxtHandler txtHandler;
        txtHandler.logCar();
    }
    double a = reserveDecimal(0.05 * car_ve.size() / prior_car_num, 5) 
                + 0.2375 * reserveDecimal(reserveDecimal(static_cast<double>(max_speed) / min_speed, 5) / reserveDecimal(static_cast<double>(max_prior_speed) / min_prior_speed, 5),5)
                + 0.2375 * reserveDecimal(reserveDecimal(static_cast<double>(max_car_start_time)/min_car_start_time,5)/reserveDecimal(static_cast<double>(max_prior_car_start_time)/prior_car_start_time,5),5)
                + 0.2375 * reserveDecimal(static_cast<double>(start.size())/prior_start.size(),5) + 0.2375 * reserveDecimal(static_cast<double>(end.size())/prior_end.size(),5);
    double b = reserveDecimal(0.8 * car_ve.size() / prior_car_num, 5) 
                + 0.05 * reserveDecimal(reserveDecimal(static_cast<double>(max_speed) / min_speed, 5) / reserveDecimal(static_cast<double>(max_prior_speed) / min_prior_speed, 5),5)
                + 0.05 * reserveDecimal(reserveDecimal(static_cast<double>(max_car_start_time)/min_car_start_time,5)/reserveDecimal(static_cast<double>(max_prior_car_start_time)/prior_car_start_time,5),5)
                + 0.05 * reserveDecimal(static_cast<double>(start.size())/prior_start.size(),5) + 0.05 * reserveDecimal(static_cast<double>(end.size())/prior_end.size(),5);
    cout<<"time consume "<<time_consume<<endl;
    cout<<"prior car time consume "<<prior_car_end_time-prior_car_start_time<<endl;
    cout<<"all car time consume "<<total_car_time<<endl;
    cout<<"all prior car time consume "<<prior_car_time<<endl;
    cout<<"finally time "<<static_cast<int>(a*(prior_car_end_time-prior_car_start_time) + time_consume)<<endl;
    cout<<"finally total time "<<static_cast<int>(b*prior_car_time+total_car_time)<<endl;
    return prior_car_end_time;
}

void Scheduler::scheduleCarInGarage(vector<int>& start_cars,int n_road_index,bool reverse){
    vector<int> temp;
    for(size_t i=0;i<start_cars.size();i++){
        Car& car = car_ve[start_cars[i]];
        int cross = car.getS_id();
        int road_index = car.getNextRoadID();
        int res = -1;
        Road& road = road_ve[road_index];
        if(road_index==n_road_index){
            if(road_ve[road_index].getS_id()==cross&&!reverse){
                res = toRoad(start_cars[i],road_index,road.car_on_road,false);
            }
            else if(road_ve[road_index].getE_id()==cross&&reverse){
                res = toRoad(start_cars[i],road_index,road.car_on_road_r,true);
            }
        }
        if(res != 0){
            temp.push_back(start_cars[i]);
        }
        else{
            car_ve[start_cars[i]].setOnRoadTime(cur_time);
        }
    }
    start_cars.swap(temp);
    temp.clear();
}

void Scheduler::scheduleCars(int ct){
    cur_time = ct;
    /* 
    第一步:调度所有道路上的车辆
     */
    for(size_t i=0;i<road_ve.size();i++){
        scheduleCarsOnRoad(i);
    }
    scheduleCarInGarage(pri_wait_cars);
    /* 
    第二步:调度所有等待车辆
     */
    all_car_end = false;
    int time_ = 0;
    //for(const auto u : dead_lock_union)if(u!=-1)throw "u!=-1";
    while(!all_car_end){
        all_car_end = true;
        vector<int> t(2*road_ve.size(),-1);
        dead_lock_union.swap(t);
        //scheduleCarInGarage(pri_wait_cars);
        for(size_t i=0;i<cross_id_inc.size();i++){
            scheduleCarsOnCross(cross_id_inc[i]);
        }
        time_++;
        if(time_%100==0)
            cout<<"time="<<time_<<endl;
        if(time_>1000){
            TxtHandler th;
            th.logRoadStatus(cur_time);
            for(size_t i=0;i<road_ve.size();i++){
                Road& r = road_ve[i];
                for(size_t j=0;j<r.car_on_road.size();j++){
                    if(!r.car_on_road[j].empty()){
                        int car_index = r.car_on_road[j][0].first;
                        if(car_ve[car_index].getWait()){
                            int sid = r.getS_id();
                            int eid = r.getE_id();
                            int next_road = car_ve[car_index].getNextRoadID();
                            Road& nr = road_ve[next_road];
                            int nsid = nr.getS_id();
                            int neid = nr.getE_id();
                            int cross;
                            if(sid==nsid||sid==neid)cross = sid;
                            else cross = eid;
                            int reverse = cross==nsid?0:1;
                            cerr<<"cross "<<cross_ve[cross].getID()<<" road "<<r.getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()
                                 <<" next road "<<road_ve[car_ve[car_index].getNextRoadID()].getID()<<" reverse "
                                 <<to_string(reverse)<<" car spd "<<car_ve[car_index].getMax_spd()
                                 <<" priorty "<<car_ve[car_index].getPriority()<<" prior "<<car_ve[car_index].getPrior()<<" still wait"<<endl;
                        }
                    }
                }
                if(r.getIs_two_way()){
                    for(size_t j=0;j<r.car_on_road_r.size();j++){
                        if(!r.car_on_road_r[j].empty()){
                            int car_index = r.car_on_road_r[j][0].first;
                            if(car_ve[car_index].getWait()){
                                int sid = r.getS_id();
                                int eid = r.getE_id();
                                int next_road = car_ve[car_index].getNextRoadID();
                                Road& nr = road_ve[next_road];
                                int nsid = nr.getS_id();
                                int neid = nr.getE_id();
                                int cross;
                                if(sid==nsid||sid==neid)cross = sid;
                                else cross = eid;
                                int reverse = cross==nsid?0:1;
                                cerr<<"cross "<<cross_ve[cross].getID()<<" reverse road "<<r.getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()
                                    <<" next road "<<road_ve[car_ve[car_index].getNextRoadID()].getID()<<" reverse "
                                    <<to_string(reverse)<<" car spd "<<car_ve[car_index].getMax_spd()
                                    <<" priorty "<<car_ve[car_index].getPriority()<<" prior "<<car_ve[car_index].getPrior()<<" still wait"<<endl;
                            }
                        }
                    }
                }
            }
            cerr<<"cur_time "<<cur_time<<endl;
            throw time_;
        }
        dead_lock_union.clear();
    }
    /* 
    test 
     */
    for(size_t i=0;i<road_ve.size();i++){
        Road& r = road_ve[i];
        for(size_t j=0;j<r.car_on_road.size();j++){
            if(!r.car_on_road[j].empty()){
                int car_index = r.car_on_road[j][0].first;
                if(car_ve[car_index].getWait()){
                    cerr<<"cur time "<<cur_time<<" road "<<road_ve[i].getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()<<" still wait"<<endl;
                }
            }
        }
        if(r.getIs_two_way()){
            for(size_t j=0;j<r.car_on_road_r.size();j++){
                if(!r.car_on_road_r[j].empty()){
                    int car_index = r.car_on_road_r[j][0].first;
                    if(car_ve[car_index].getWait()){
                        cerr<<"cur time "<<cur_time<<" reverse road "<<road_ve[i].getID()<<" lane "<<j<<" car index "<<car_ve[car_index].getID()<<" still wait"<<endl;
                    }
                }
            }
        }
    }
    scheduleCarInGarage(pri_wait_cars);
    //cout<<"wait cars size "<<wait_cars.size()<<endl;
    if(!wait_cars.empty())scheduleCarInGarage(wait_cars);
}

bool Scheduler::scheduleOneCar(int car_index,int road_index,bool reverse){
    int res = -1;
    Road& r = road_ve[road_index];
    //print = true;
    if(reverse)res = toRoad(car_index,road_index,r.car_on_road_r,reverse);
    else res = toRoad(car_index,road_index,r.car_on_road,reverse);
    bool b = res==0?true:false;
    return b;
}