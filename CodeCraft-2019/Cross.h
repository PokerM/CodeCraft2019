#ifndef CROSS_H
#define CROSS_H
#include <ostream>
#include <map>

using namespace std;

extern const int w;
extern const int h;
extern std::map<int,int> car_map;
extern std::map<int,int> road_map;
extern std::map<int,int> cross_map;
class Cross{
public:
    Cross(int id, int r[4])
    :id(id),diff(INT32_MAX){
        for(int i=0;i<4;i++)road[i]=r[i];
    };
    int getID(){return id;};
    int getX(){return x;}
    int getY(){return y;}
    const int* getRoadID(){return road;};
    void setRoadID(int* r){
        for(int i=0;i<4;i++){
            if(r[i]!=-1){
                road[i] = r[i];
            }
        }
    }
    void printRoad(){
        /* cout<<"cross:";
        for(int i=0;i<4;i++){
            if(road[i]==-1)cout<<road[i]<<" ";
            else cout<<road_ve[road[i]].getID()<<" ";
        }
        cout<<endl; */
    }
    void setLocation(int xx,int yy){
        x = xx;
        y = yy;
    }
    void setDiff(int d){
        if(diff!=INT32_MAX&&diff!=d){
            cerr<<"cross:wrong diff "<<diff<<" "<<d<<endl;
        }
        //diff = d;
        // if(diff != d){
        //     diff = d;
        // }
        diff = d;
    }
    void rotate(){
        if(diff<0)diff+=4;
        if(diff==0)return;
        for(int i=0;i<diff;i++){
            for(size_t j=0;j<3;j++){
                int t = road[j];
                road[j] = road[3];
                road[3] = t;
            }
        }
    }
    // void addCar(int car_index,bool left){
    //     if(left)turn_left_set.insert(car_index);
    //     else turn_right_set;
    // }
    // void deleCar(int car_index,bool left){
    //     if(left){
    //         if(turn_left_set.find(car_index)==turn_left_set.end()){
    //             cerr<<"no car turn left!"<<endl;
    //             throw car_index;
    //         }
    //         turn_left_set.erase(car_index);
    //     }
    //     else{
    //         if(turn_right_set.find(car_index)==turn_right_set.end()){
    //             cerr<<"no car turn right!"<<endl;
    //             throw car_index;
    //         }
    //         turn_right_set.erase(car_index);
    //     }
    // }
    bool canGo(bool left){
        if(left){
            return turn_right_set.empty();
        }
        return turn_left_set.empty();
    }
    vector<int> ordinaryWaitCar;
    vector<int> priorNotPresetWaitCar;
private:
    int id;
    int road[4];
    int x;
    int y;
    int diff;
    set<int> turn_left_set;
    set<int> turn_right_set;
};

#endif