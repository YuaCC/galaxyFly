#ifndef _GALAXYFLY_HPP_
#define _GALAXYFLY_HPP_

#include "anynet.hpp"


class GalaxyFly : public AnyNet {
private:
    void djistra(int src_router,int local_hop_limit,int global_hop_limit);
protected:
  virtual void buildRoutingTable();
  Configuration cfg ;
  int _router_port_cnt;
  int _galaxyfly_n,_galaxyfly_q,_galaxyfly_a,_galaxyfly_p;
public:
  GalaxyFly( const Configuration &config, const string & name );

  static void RegisterRoutingFunctions();
  void Display(ostream &os);
};

void nomin_galaxyfly( const Router *r, const Flit *f, int in_channel,
		      OutputSet *outputs, bool inject );


#endif
