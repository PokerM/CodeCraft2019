#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <set>
#include <iostream>
#include <set>

class Road;
class Cross;

using namespace std;

    /* 
    邻近路口,可以通过一条道路直接到达的路口
     */
    struct Adjacent_cross
    {
        int cross_id;//下一节点ID
        int road_id;//连接道路ID
        double road_w;//道路权重,道路长度/min(道路限速,当前车速)
        Adjacent_cross(int c_id,int r_id,double r_w):cross_id(c_id),road_id(r_id),road_w(r_w){};
    };
    struct Path{
        int cross_id;//下一节点ID
        int road_id;//连接道路ID
        Path():cross_id(0),road_id(0){};
        Path(int c_id,int r_id):cross_id(c_id),road_id(r_id){};
    };
    struct Path_with_weight
    {
        vector<Path> path_vector;
        double weight;
        Path_with_weight():path_vector(),weight(0){};
    };
class Graph{
public:
    Graph(int cross_num, const int spd, vector<bool>& active_cross):spd(spd),path_num(90),cross_num(cross_num),active_cross(active_cross),
                                  adj_cross(cross_num,vector<Adjacent_cross>()),
                                  path_map(cross_num,vector<vector<Path_with_weight> >()){
    };//90
    void addEdge(const Road& r);
    void addEdge(vector<Road>& road_ve);
    void getConnectedNode(int c, vector<tuple<int, int,double>>& nodes);
    void dijkstra();
    void printEdges();
    int getSpd(){return spd;}
    const vector<Path_with_weight>& getPath(int s_id,int e_id) const{
        return path_map[s_id][e_id];
    }
    const vector<vector<Path_with_weight> >& getPath(int s_id) const{
        return path_map[s_id];
    }
    ~Graph(){};
private:
    const int spd;
    const size_t path_num;//每两个点之间的路径数
    int cross_num;
    const vector<bool> active_cross;
    vector<vector<Adjacent_cross> > adj_cross;//连接图,<int,int,double>代表节点ID 道路ID 权重
    vector<vector<vector<Path_with_weight> > > path_map;//最小路径map,根据节点ID索引
};
#endif