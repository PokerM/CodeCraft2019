#include "Graph.h"
#include <iostream>
#include "Road.h"
#include "Cross.h"
using namespace std;

void Graph::addEdge(const Road& r){
    int s = r.getS_id();
    int e = r.getE_id();
    bool bi = r.getIs_two_way();
    int len = r.getLen();
    int limit_spd = r.getLimit_spd();
    int id = r.getID();
    limit_spd = limit_spd<spd?limit_spd:spd;
    double t = static_cast<double>(len)/limit_spd;
    (adj_cross[s]).push_back(Adjacent_cross(e,id,t));
    if(bi)adj_cross[e].push_back(Adjacent_cross(s,id,t));
    return;
}

void Graph::addEdge(vector<Road>& road_ve){
    for(size_t i=0;i<road_ve.size();i++){
        Road& r = road_ve[i];
        int s = r.getS_id();
        int e = r.getE_id();
        bool bi = r.getIs_two_way();
        int len = r.getLen();
        int limit_spd = r.getLimit_spd();
        int id = i;
        limit_spd = limit_spd<spd?limit_spd:spd;
        double t = static_cast<double>(len)/limit_spd;
        (adj_cross[s]).push_back(Adjacent_cross(e,id,t));
        if(bi)adj_cross[e].push_back(Adjacent_cross(s,id,t));
    }
}
void Graph::printEdges(){
    //cout<<"print path:"<<cross_num<<endl;
    for(int i=0;i<this->cross_num;i++){
        if(active_cross[i]){
            cout<<"cross_id="<<i<<" ";
            auto ve_pww = path_map[i];
            for(size_t j=0;j<ve_pww.size();j++){
                for(size_t k=0;k<ve_pww[j].size();k++){
                    auto ve = ve_pww[j][k].path_vector;
                    if(!ve.empty()){
                        cout<<"link to cross "<<j<<" with weight="<<ve_pww[j][k].weight<<" ";
                        for(const auto p:ve){
                            cout<<"->"<<p.road_id;
                        }
                    }
                    cout<<endl;
                }
            }
            cout<<endl;
            cout<<"-------------------------"<<endl;
        }
    }
}
void Graph::dijkstra(){
    for(int i=0;i<cross_num;i++){
        if(active_cross[i]){
            vector<bool> S(cross_num,false);
            vector<bool> U(cross_num,true);
            S[i] = true;
            U[i] = false;
            vector<vector<Path_with_weight>> vvwp(cross_num,vector<Path_with_weight>());
            int found_num = 1;//已找到路径节点计数器
            int current_cross = i;//当前节点ID            
            vvwp[current_cross].push_back(Path_with_weight());
            while(found_num<cross_num){
                vector<Path_with_weight>& vpw = vvwp[current_cross];//当前路径集合
                auto adj_cur = adj_cross[current_cross];//当前节点连接列表,节点ID 道路ID 权重
                /* 
                松弛节点
                 */
                for(const auto node : adj_cur){//遍历所有相邻节点
                    if(U[node.cross_id]){
                        //cout<<vpw.size()<<endl;
                        for(const auto pw : vpw){//遍历所有当前节点路径
                            double current_w = pw.weight;
                            auto current_path = pw.path_vector;
                            vector<Path_with_weight>& vpww = vvwp[node.cross_id];
                            current_w += node.road_w;
                            current_path.push_back(Path(node.cross_id,node.road_id));
                            Path_with_weight pww;
                            pww.weight = current_w;
                            pww.path_vector = current_path;
                            bool inserted = false;
                            for(size_t i=0;i<vpww.size();i++){
                                if(current_w < vpww[i].weight){
                                    inserted = true;
                                    vpww.insert(vpww.begin()+i,pww);
                                    break;
                                }
                            }
                            if(!inserted)vpww.push_back(pww);
                            if(vpww.size()>path_num)
                                    vpww.erase(vpww.begin()+path_num,vpww.end());
                        }
                    }
                }
                double min_ = INT32_MAX;
                /* 
                得到下一节点
                 */
                for(int j=0;j<cross_num;j++){
                    //cout<<vvwp.size()<<endl;
                    if(U[j]&&!vvwp[j].empty()&&(vvwp[j][0]).weight<min_){
                        min_ = (vvwp[j][0]).weight;
                        current_cross = j;
                    }
                }
                //cout<<"min weight="<<min_<<endl;
                U[current_cross] = false;
                S[current_cross] = true;
                found_num++;
            }
            this->path_map[i]=vvwp;  
        }
    }
}



