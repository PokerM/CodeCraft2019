#ifndef ANSWER_H
#define ANSWER_H

#include <vector>

using namespace std;
class Answer{
private:
    int car_id;
    int start_time;
public:
    Answer(int car_id, int start_time, vector<int>& path)
    :car_id(car_id), start_time(start_time),path(path){};
    int getCarID(){return car_id;};
    int getStartTime(){return start_time;};
    vector<int> path;
};
#endif