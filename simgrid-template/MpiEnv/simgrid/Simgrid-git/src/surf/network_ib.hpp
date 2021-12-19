/* Copyright (c) 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SURF_NETWORK_IB_HPP_
#define SURF_NETWORK_IB_HPP_

#include "network_smpi.hpp"
class IBNode;


class ActiveComm{
public :
  //IBNode* origin;
  IBNode* destination;
  NetworkActionPtr action;
  double init_rate;
  ActiveComm() : destination(NULL),action(NULL),init_rate(-1){};
  ~ActiveComm(){};
};

class IBNode{
public :
  int id;
    //store related links, to ease computation of the penalties
  std::vector<ActiveComm*> ActiveCommsUp;
  //store the number of comms received from each node
  std::map<IBNode*, int> ActiveCommsDown;
  //number of comms the node is receiving
  int nbActiveCommsDown;
  IBNode(int id) : id(id),nbActiveCommsDown(0){};
  ~IBNode(){};
};

class NetworkIBModel : public NetworkSmpiModel {
private:
  void updateIBfactors_rec(IBNode *root, bool* updatedlist);
  void computeIBfactors(IBNode *root);
public:
  NetworkIBModel();
  NetworkIBModel(const char *name);
  ~NetworkIBModel();
  void updateIBfactors(NetworkActionPtr action, IBNode *from, IBNode * to, int remove);
  
  xbt_dict_t active_nodes;
  std::map<NetworkActionPtr , std::pair<IBNode*,IBNode*> > active_comms;
  
  double Bs;
  double Be;
  double ys;

};

#endif
