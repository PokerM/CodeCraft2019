#ifndef ROAD_H
#define ROAD_H
#include <queue>
#include <vector>
#include <iostream>
#include "Car.h"
#include <set>
#include <map>

using namespace std;


extern set<int> open_road;
extern map<int,int> road_map;
extern int on_road_car_num;
extern int road_num;
class Road{
private:
    bool is_two_way;
    bool add;
    bool add_r;
    const int id;
    const int len;
    const int limit_spd;
    const int lane_num;
    int s_id;
    int e_id;
    int car_num;
    int car_num_r;
    int real_car_num;
    int real_car_num_r;
    int max_speed;
    int r_max_speed;
    const int load_limit;
    int priority;
    const double load_factor;
    double cur_factor,cur_factor_r;
    set<int> car_set;
    set<int> r_car_set;
    set<int> preset_car_set;
    set<int> r_preset_car_set;
    map<int,int> car_speed_counter;
    map<int,int> r_car_speed_counter;
public:
    Road(int id,int len, int limit_spd, int lane_num, int s_id, int e_id, bool is_two_way)
    :is_two_way(is_two_way),add(true),add_r(true),id(id),len(len),limit_spd(limit_spd),lane_num(lane_num),s_id(s_id),e_id(e_id),car_num(0),car_num_r(0),
    real_car_num(0),real_car_num_r(0),max_speed(0),r_max_speed(0),
    load_limit(len*lane_num),priority(0),load_factor(15),cur_factor(0),
    car_on_road(lane_num,vector<pair<int,int> >()),
    car_on_road_r(lane_num,vector<pair<int,int> >()){
        if(is_two_way)cur_factor_r = 0;
        else cur_factor_r = 1;
    };
    bool getAdd(bool reverse){
        if(reverse)return add_r;
        return add;
    };
    void addCar(bool reverse,int s){
        if(reverse){
            car_num_r++;
            if(r_car_speed_counter.find(s)==r_car_speed_counter.end()){
                r_car_speed_counter[s] = 1;
                if(s>r_max_speed)r_max_speed=s;
            }
            else{
                r_car_speed_counter[s]++;
            }
            if(open_road.find(road_num+road_map[id])!=open_road.end()){
                open_road.erase(road_num+road_map[id]);
            }
           if(car_num_r>=load_limit*load_factor)add_r=false;
            /*map<int,int>::reverse_iterator it = r_car_speed_counter.rbegin();
            for(;it!=r_car_speed_counter.rend();it++){
                if(it->second!=0){
                    r_max_speed = it->first;
                    break;
                }
            } */
        }
        else{
            car_num++;
            if(car_speed_counter.find(s)==car_speed_counter.end()){
                car_speed_counter[s] = 1;
                if(s>max_speed)max_speed=s;
            }
            else{
                car_speed_counter[s]++;
            }
            if(open_road.find(road_map[id])!=open_road.end()){
                open_road.erase(road_map[id]);
            }
        }
    }
    void deleteCar(bool reverse,int s){
        //upDateFactor(false,reverse);
        double max_num = load_factor*load_limit;
        if(reverse){
            car_num_r--;
            if(car_num_r==0){
                open_road.insert(road_map[id]+road_num);
            }
            if(car_num_r<max_num)add_r=true;
            // if(r_car_speed_counter.find(s)==r_car_speed_counter.end()){
            //     cerr<<"no speed="<<s<<" car!"<<endl;
            //     throw s;
            // }
            // else{
            //     r_car_speed_counter[s]--;
            //     if(s==r_max_speed&&r_car_speed_counter[s]==0){
            //         map<int,int>::reverse_iterator it = r_car_speed_counter.rbegin();
            //         for(;it!=r_car_speed_counter.rend();it++){
            //             if(it->second!=0){
            //                 r_max_speed = it->first;
            //                 break;
            //             }
            //         }
            //     }
            // }
        }
        else{
            car_num--;
            if(car_num==0){
                open_road.insert(road_map[id]+road_num);
            }
            if(car_num<max_num)add=true;
            // if(car_speed_counter.find(s)==car_speed_counter.end()){
            //     cerr<<"no speed="<<s<<" car!"<<endl;
            //     throw s;
            // }
            // else{
            //     car_speed_counter[s]--;
            //     if(s==max_speed&&car_speed_counter[s]==0){
            //         map<int,int>::reverse_iterator it = car_speed_counter.rbegin();
            //         for(;it!=car_speed_counter.rend();it++){
            //             if(it->second!=0){
            //                 max_speed = it->first;
            //                 break;
            //             }
            //         }
            //     }
            // }
        }
    }
    void addCar(int car_index,bool reverse){
        //if(car_index==13121)cerr<<"car added"<<endl;
        if(!reverse){
            if(car_set.find(car_index)!=car_set.end()){
                cerr<<"car already in set "<<car_index<<endl;
                throw car_index;
            }
            else{
                car_set.insert(car_index);
                if((open_road.find(road_map[id])!=open_road.end())){
                    open_road.erase(road_map[id]);
                }
            }
        }
        else{
            if(r_car_set.find(car_index)!=r_car_set.end()){
                cerr<<"car already in set "<<car_index<<endl;
                throw car_index;
            }
            else{
                r_car_set.insert(car_index);
                if((open_road.find(road_map[id]+road_num)!=open_road.end())){
                    open_road.erase(road_map[id]+road_num);
                }
            }
        }
    }
    void addPresetCar(int car_index,bool reverse){
        if(reverse){
            if(r_preset_car_set.find(car_index)!=r_preset_car_set.end()){
                cerr<<"car already in set "<<car_index<<endl;
                throw car_index;
            }
            else{
                r_preset_car_set.insert(car_index);
            }
        }
    }
    void addCar(int car_index,bool reverse,bool preset){
        if(reverse){
            addCar(car_index,r_car_set);
            if(preset)addCar(car_index,r_preset_car_set);
        }
        else{
            addCar(car_index,car_set);
            if(preset)addCar(car_index,preset_car_set);
        }
    }
    void deleteCar(int car_index,bool reverse,bool preset){
        upDateFactor(false,reverse);
        if(reverse){
            deleteCar(car_index,r_car_set);
            if(preset)deleteCar(car_index,r_preset_car_set);
        }
        else{
            deleteCar(car_index,car_set);
            if(preset)deleteCar(car_index,preset_car_set);
        }
    }
    void addCar(int car_index,set<int>& car_set){
        if(car_set.find(car_index)!=car_set.end()){
            cerr<<"car already in set "<<car_index<<endl;
            throw car_index;
        }
        else{
            car_set.insert(car_index);
        }
    }
    void deleteCar(int car_index,bool reverse){
        upDateFactor(false,reverse);
        if(reverse){
            if(r_car_set.find(car_index)==r_car_set.end()){
                cerr<<"car not on road "<<car_index<<endl;
                cerr<<(car_set.find(car_index)==car_set.end())<<endl;
                throw car_index;
            }
            else{
                r_car_set.erase(car_index);
                if(r_car_set.empty()){
                    open_road.insert(road_map[id]+road_num);
                }
            }
        }
        else{
            if(car_set.find(car_index)==car_set.end()){
                cerr<<"car not on road "<<car_index<<endl;
                cerr<<(r_car_set.find(car_index)==r_car_set.end())<<endl;
                throw car_index;
            }
            else{
                car_set.erase(car_index);
                if(car_set.empty()){
                    open_road.insert(road_map[id]);
                }
            }
        }
    }
    void deleteCar(int car_index,set<int>& car_set){
        if(car_set.find(car_index)==car_set.end()){
            cerr<<"car not on road "<<car_index<<endl;
            throw car_index;
        }
        else{
            car_set.erase(car_index);
        }
    }
    int getCarNum(bool reverse){
        if(reverse)return car_num_r;
        return car_num;
    }
    double getBusyFactor(bool reverse){
        if(reverse)return ((double)r_car_set.size())/load_limit;
        return ((double)car_set.size())/load_limit;
    }
    double getPresetBusyFactor(bool reverse){
        if(reverse)return ((double)r_preset_car_set.size())/load_limit;
        return ((double)preset_car_set.size())/load_limit;
    }
    int getLoadLimit(){return load_limit;}
    int getID()const{return id;};
    int getLen()const{return len;};
    int getLimit_spd()const{return limit_spd;};
    int getLane_num()const{return lane_num;};
    int getS_id()const{return s_id;};
    int getE_id()const{return e_id;};
    int getIs_two_way()const{return is_two_way;};
    double getCurFactor(bool reverse){
        if(reverse)return cur_factor_r;
        return cur_factor;
    }
    vector<vector<pair<int,int> > > car_on_road;
    vector<vector<pair<int,int> > > car_on_road_r;
    void init(){
        vector<vector<pair<int,int> > > t(lane_num,vector<pair<int,int> >()),tr(lane_num,vector<pair<int,int> >());
        car_on_road.swap(t);
        car_on_road_r.swap(tr);
        t.clear();
        tr.clear();
        car_num = 0;
        car_num_r = 0;
        add = true;
        add_r = true;
        car_num = 0;
        car_num_r = 0;
        real_car_num = 0;
        real_car_num_r = 0;
        max_speed = 0;
        r_max_speed = 0;
        cur_factor = 0;
        if(is_two_way)cur_factor_r = 0;
        else cur_factor_r = 1;
        car_set.clear();
        r_car_set.clear();
        preset_car_set.clear();
        r_preset_car_set.clear();
    }
    void setSID(int sid){
        s_id = sid;
    }
    void setEID(int eid){
        e_id = eid;
    }
    void upDateFactor(bool add,bool reverse){
        if(reverse){
            if(add)real_car_num_r++;
            else real_car_num_r--;
            int num=0;
            for(size_t i=0;i<car_on_road_r.size();i++){
                num+=car_on_road_r[i].size();
            }
            if(num==0){
                cur_factor_r = 0;
            }
            else cur_factor_r = static_cast<double>(num)/load_limit;
            /* if(num!=real_car_num_r){
                cerr<<"num "<<num<<" real car num "<<real_car_num_r<<endl;
                throw num;
            } */
            //cout<<"update factor "<<cur_factor_r<<endl;
        }
        else{
            if(add)real_car_num++;
            else real_car_num--;
            int num=0;
            for(size_t i=0;i<car_on_road.size();i++){
                num+=car_on_road[i].size();
            }
            if(num==0){
                cur_factor = 0;
            }
            else cur_factor = static_cast<double>(num)/load_limit;
            /* if(num!=real_car_num){
                cerr<<"num "<<num<<" real car num "<<real_car_num<<endl;
                throw num;
            } */
            //cout<<"update factor "<<cur_factor<<endl;
        }
    }
    int getMaxSpeed(bool reverse){
        if(reverse)return r_max_speed;
        return max_speed;
    }
};

#endif