#include "iostream"
#include <vector>
#include <fstream>
#include <set>
#include <algorithm>
#include <time.h>
#include <sys/time.h>

#include "TxtHandler.h"
#include "Car.h"
#include "Cross.h"
#include "Road.h"
#include "Answer.h"
#include "Scheduler.h"
#include "Graph.h"


using namespace std;


vector<Car> car_ve;
vector<Road> road_ve; 
vector<Cross> cross_ve;
map<int,int> car_map;
map<int,int> road_map;
map<int,int> cross_map;
const bool debug = false;

int main(int argc, char *argv[])
{
    std::cout << "Begin" << endl;
	
	if(argc < 6){
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << endl;
		exit(1);
	}
	
	string carPath(argv[1]);
	string roadPath(argv[2]);
	string crossPath(argv[3]);
	string presetAnswerPath(argv[4]);
	string answerPath(argv[5]);
	
	std::cout << "carPath is " << carPath << endl;
	std::cout << "roadPath is " << roadPath << endl;
	std::cout << "crossPath is " << crossPath << endl;
	std::cout << "presetAnswerPath is " << presetAnswerPath << endl;
	std::cout << "answerPath is " << answerPath << endl;
	
	// TODO:read input filebuf
	ofstream ofs;
    //ofs.open(answerPath,ofstream::out);
    ofs.close();
    
    vector<Answer> answer_ve;

    TxtHandler th;
    th.getCarFromTxt(car_ve,carPath);
    th.getRoadFromTxt(road_ve,roadPath);
    th.getCrossFromTxt(cross_ve,crossPath);
	th.getAnswerFromTxt(answer_ve,presetAnswerPath);
    th.getAnswerFromTxt(answer_ve,answerPath);
    th.reMap(car_ve,road_ve,cross_ve);	
    std::cout<<"car num "<<car_ve.size()<<endl;
    std::cout<<"cross_num  "<<cross_ve.size()<<endl;
    std::cout<<"road_num "<<road_ve.size()<<endl;
	std::cout<<"answer num "<<answer_ve.size()<<endl;

    clock_t startTime,endTime;
	startTime = clock();
    
    Scheduler scheduler;
    scheduler.scheduleAllCars(answer_ve);

    endTime = clock();
	cout << "Total Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
	return 0;
}