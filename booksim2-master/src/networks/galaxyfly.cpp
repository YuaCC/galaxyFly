// $Id$

/*
 Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*anynet
 *
 *Network setup file format
 *example 1:
 *router 0 router 1 15 router 2
 *
 *Router 0 is connect to router 1 with a 15-cycle channel, and router 0 is connected to
 * router 2 with a 1-cycle channel, the channels latency are unidirectional, so channel
 * from router 1 back to router 0 is only single-cycle because it was not specified
 *
 *example 2:
 *router 0 node 0 node 1 5 node 2 5
 *
 *Router 0 is directly connected to node 0-2. Channel latency is 5cycles for 1 and 2. In
 * this case the latency specification is bidirectional, the injeciton and ejection lat
 * for node 1 and 2 are 5-cycle
 *
 *other notes:
 *
 *Router and node numbers must be sequential starting with 0
 *Credit channel latency follows the channel latency, even though it travels in revse
 * direction this might not be desired
 *
 */

#include "galaxyfly.hpp"
//#define DEBUG

//[0][src router][dest node]=(port, latency)
//[1][src router][dest router]=(port, latency)
vector<map<int,  map<int, pair<int,int> > > > global_router_list;
//_global_galaxyfly_distance_table[src_router][dest_router][global_link_to_use][local_link_to_use]=hop_cnt
int ****_global_galaxyfly_distance_table=NULL;
int global_hop_limit=2;
int local_hop_limit=3;
int galaxyfly_n,galaxyfly_q,galaxyfly_a,galaxyfly_p;
GalaxyFly::GalaxyFly( const Configuration &config, const string & name )
    :  AnyNet( config, name )
{
    cfg = config;
    _router_port_cnt =0;
    for(int i =0; i<_size; ++i)
    {
        int ports_cnt = router_list[0][i].size() + router_list[1][i].size();
        _router_port_cnt = _router_port_cnt>ports_cnt?_router_port_cnt:ports_cnt;
    }
    buildRoutingTable();
}


void GalaxyFly::RegisterRoutingFunctions()
{
    cout<<"XXXXXXXXXXXX  GalaxyFly::RegisterRoutingFunctions  XXXXXXXXXXXXXXXXX"<<endl;
    gRoutingFunctionMap["min_galaxyfly"] = &min_anynet;
    gRoutingFunctionMap["nomin_galaxyfly"] = &nomin_galaxyfly;
}



void GalaxyFly::buildRoutingTable()
{
    cout<<"XXXXXXXXXXXX  GalaxyFly::buildRoutingTable  XXXXXXXXXXXXXXXXX"<<endl;
    _galaxyfly_n= cfg.GetInt("galaxyfly_n");
    _galaxyfly_q= cfg.GetInt("galaxyfly_q");
    _galaxyfly_a= cfg.GetInt("galaxyfly_a");
    _galaxyfly_p= cfg.GetInt("galaxyfly_p");
    galaxyfly_n = _galaxyfly_n;
    galaxyfly_q = _galaxyfly_q;
    galaxyfly_a = _galaxyfly_a;
    galaxyfly_p = _galaxyfly_p;
    global_router_list = router_list;
#ifdef DEBUG
    for(map<int,int>::iterator it = node_list.begin(); it!= node_list.end(); ++it)
        cout<<"node "<<it->first<<" router "<<it->second<<endl;
    map<int,  map<int, pair<int,int> > > router_list_debug_0 = router_list[0];
    for (map<int,  map<int, pair<int,int> > > :: iterator it = router_list_debug_0.begin(); it!=router_list_debug_0.end(); ++it)
    {
        int router_a = it->first;
        cout<<"router "<<router_a<<" connected to"<<endl;
        map<int, pair<int,int> > *router_list_b = &(it->second);
        for (map<int, pair<int,int> >::iterator itb= router_list_b->begin(); itb != router_list_b->end(); ++itb)
        {
            int node = itb->first;
            int port =  itb->second.first;
            int latency = itb->second.second;
            cout<<"\tnode "<<node<<" port "<<port<<" latency "<<latency<<endl;
        }
    }
    map<int,  map<int, pair<int,int> > > router_list_debug_1 = router_list[1];
    for (map<int,  map<int, pair<int,int> > > :: iterator it = router_list_debug_1.begin(); it!=router_list_debug_1.end(); ++it)
    {
        int router_a = it->first;
        cout<<"router "<<router_a<<" connected to"<<endl;
        map<int, pair<int,int> > *router_list_b = &(it->second);
        for (map<int, pair<int,int> >::iterator itb= router_list_b->begin(); itb != router_list_b->end(); ++itb)
        {
            int router_b = itb->first;
            int port =  itb->second.first;
            int latency = itb->second.second;
            cout<<"\trouter "<<router_b<<" port "<<port<<" latency "<<latency<<endl;
        }
    }
#endif //
    _global_galaxyfly_distance_table =new int ***[_size];
    for (int i=0; i<_size; ++i)
    {
        _global_galaxyfly_distance_table[i] =new int **[_size];
        for(int j=0; j<_size; ++j)
        {
            _global_galaxyfly_distance_table[i][j] =new  int *[global_hop_limit+1];
            for(int k=0; k<global_hop_limit+1; ++k)
            {
                _global_galaxyfly_distance_table[i][j][k] =new   int [local_hop_limit+1];
                memset(_global_galaxyfly_distance_table[i][j][k],0x1f,(local_hop_limit+1)*sizeof(int));
            }
        }
    }
    for (map<int,  map<int, pair<int,int> > > :: iterator it = router_list[1].begin(); it!=router_list[1].end(); ++it)
    {
        int router_a = it->first;
        int supernode_a = router_a / _galaxyfly_a;
        map<int, pair<int,int> > *router_list_b = &(it->second);
        for (map<int, pair<int,int> >::iterator itb= router_list_b->begin(); itb != router_list_b->end(); ++itb)
        {
            int router_b = itb->first;
            int supernode_b = router_b / _galaxyfly_a;
            //int port =  itb->second.first;
            //int latency = itb->second.second;
            if (supernode_a == supernode_b)
                _global_galaxyfly_distance_table[router_a][router_b][0][1]  =1;
            else
                _global_galaxyfly_distance_table[router_a][router_b][1][0]  =1;
        }
        for(int i =0;i<=global_hop_limit;++i)
            for(int j=0;j<=global_hop_limit;++j)
                _global_galaxyfly_distance_table[router_a][router_a][i][j]  =0;
    }
    //floyd
    for(int k=0; k<_size; ++k)
        for (int k_global_hop =0; k_global_hop<=global_hop_limit; ++k_global_hop)
            for (int k_local_hop =0; k_local_hop<=local_hop_limit; ++k_local_hop)
                for (int i=0; i<_size; ++i)
                    for (int i_global_hop =global_hop_limit; i_global_hop>=k_global_hop; --i_global_hop)
                        for (int i_local_hop =local_hop_limit; i_local_hop>=k_local_hop; --i_local_hop)
                            for(int j=0; j<_size; ++j)
                            {
                                int dis_ik = _global_galaxyfly_distance_table[i][k][i_global_hop-k_global_hop][i_local_hop-k_local_hop];
                                int dis_kj = _global_galaxyfly_distance_table[k][j][k_global_hop][k_local_hop];
                                int dis_ij = _global_galaxyfly_distance_table[i][j][i_global_hop][i_local_hop];
                                #ifdef DEBUG
                                    cout<<" i "<<i<<" global_hop "<<i_global_hop<<" local_hop "<<i_local_hop<<endl;
                                    cout<<" k "<<k<<" global_hop "<<k_global_hop<<" local_hop "<<k_local_hop<<endl;
                                    cout<<" j "<<j<<endl;
                                    cout<<"dis_ik "<<dis_ik<<" dis_kj "<<dis_kj<<" dis_ij "<<dis_ij<<endl<<endl;
                                #endif // DEBUG
                                if (dis_ij>dis_ik+dis_kj) _global_galaxyfly_distance_table[i][j][i_global_hop][i_local_hop] = dis_ik+dis_kj ;

                            }


};


void nomin_galaxyfly( const Router *r, const Flit *f, int in_channel,
                    OutputSet *outputs, bool inject )
{
    int out_port=-1;
    int out_vc = -1;
    if(inject)
    {
        f->ph = (global_hop_limit << 4) | local_hop_limit;
        f->intm = -1;
        out_port=-1;
        out_vc = rand()%gNumVCs;
    }
    else
    {
        int dst_router = f->dest / galaxyfly_p;
        int src_router = r->GetID();
        int src_supernode= src_router / galaxyfly_a;
        //int dst_supernode= dst_router / galaxyfly_a;
        int global_hop  = (f->ph &0xf0)>>4;
        int local_hop  =  f->ph &0x0f;

        if (src_router == dst_router) //at the dst router
        {
            out_port = global_router_list[0][src_router][f->dest].first;
            out_vc = rand()%gNumVCs;

        }
        else if(_global_galaxyfly_distance_table[src_router][dst_router][global_hop][local_hop] ==1)  //use min routing if src_router  and dst_router are connected directly
        {
            map<int, pair<int,int> > *router_list_src= &global_router_list[1][src_router];
            for (map<int, pair<int,int> >::iterator it= router_list_src->begin(); it != router_list_src->end(); ++it)
            {
                int other_router  = it->first;
                int other_supernode = other_router / galaxyfly_a;
                if ((other_supernode == src_supernode && local_hop>=1) || (other_supernode != src_supernode && global_hop>=1))
                {
                    int other_port =  it->second.first;
                    if (other_router == dst_router)
                    {
                        out_port = other_port;
                        if (other_supernode == src_supernode) out_vc = --local_hop;  else out_vc = --global_hop;
                        f->ph = (global_hop << 4) | local_hop;
                        break;
                    }
                }
            }
        }
        else
        {
            int distance = _global_galaxyfly_distance_table[src_router][dst_router][global_hop][local_hop];
            map<int, pair<int,int> > *router_list_src= &global_router_list[1][src_router];
            int best_min_port = -1;
            int best_nomin_port = -1;
            int best_min_supernode = -1;
            int best_queue_size_min = -1;
            int best_queue_size_nomin = -1;
            for (map<int, pair<int,int> >::iterator it= router_list_src->begin(); it != router_list_src->end(); ++it)
            {
                int other_router  = it->first;
                int other_supernode = other_router / galaxyfly_a;
                int other_port =  it->second.first;
                int other_distance;
                if (other_supernode == src_supernode)other_distance = _global_galaxyfly_distance_table[other_router][dst_router][global_hop][local_hop-1];
                else other_distance = _global_galaxyfly_distance_table[other_router][dst_router][global_hop-1][local_hop];
                int other_queue_size = max(r->GetUsedCredit(other_port), 0) ;

                if(other_distance < distance &&(best_min_port ==-1 || other_queue_size< best_queue_size_min)){
                    best_min_port = other_port;
                    best_queue_size_min = other_queue_size;
                    best_min_supernode = other_supernode;
                }
                if(other_distance == distance  &&(best_nomin_port ==-1 || other_queue_size< best_queue_size_nomin)){
                    best_nomin_port = other_port;
                    best_queue_size_nomin = other_queue_size;
                }
            }
            if (f->hops >0 || best_nomin_port ==-1 ||best_queue_size_nomin*2+10 > best_queue_size_min){
                out_port = best_min_port;
                if (best_min_supernode == src_supernode) out_vc =--local_hop;else out_vc =--global_hop;
            }
            else{
                out_port = best_nomin_port;
                out_vc = gNumVCs -1 ;
            }

            f->ph = (global_hop << 4) | local_hop;


        }
        }
        outputs->Clear( );
        outputs->AddRange( out_port, out_vc, out_vc );
    }

