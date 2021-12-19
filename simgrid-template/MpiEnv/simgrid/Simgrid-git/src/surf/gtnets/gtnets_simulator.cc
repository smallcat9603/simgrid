/* Copyright (c) 2007-2011, 2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "gtnets_simulator.h"
#include "gtnets_topology.h"
#include <map>
#include <vector>
#ifdef XBT_DEBUG
	#undef XBT_DEBUG
#endif
#include "xbt/log.h"
#include "xbt/asserts.h"
#include "RngStream.h"

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_network_gtnets_simulator, surf_network_gtnets,
                                "Logging specific to the SURF network GTNetS simulator");


using namespace std;

static vector<void*> meta_flows;
static int* meta_nflow;
static int meta_flg = 0;


void static tcp_sent_callback(void* action, double completion_time);


// Constructor.
// TODO: check the default values.
GTSim::GTSim(int WindowSize){
  int wsize = WindowSize;
  is_topology_ = 0;
  nflow_ = 0;
  jitter_ = 0;
  jitter_seed_ = 10;

  // EXTRACTED FROM GTNETS SOURCE CODE COMMENTS
  //  REDQueue::REDQueue(
  //     DCount_t in_w_q, Count_t in_min_th, Count_t in_max_th,
  //     Count_t in_limit, DCount_t in_max_p, Count_t in_mean_pktsize) : iface(nil)
  // Set default values.
  //Doc:Desc This constructor the critical RED parameters and builds a
  //Doc:Desc correspoding RED queue
  //Doc:Arg1 weight of the queue
  //Doc:Arg2 minimum threshold
  //Doc:Arg3 maximum threshold
  //Doc:Arg4 Limit/max size for the queue
  //Doc:Arg5 maximum value for mark/drop probability
  //Doc:Arg6 Average packet size

  //Default Parameters
  //REDQueue *default_red_queue_ = new REDQueue(0.002, 2500, 7500, 30000, 0.10, 500);
  //Same as above
  //REDQueue *default_red_queue_ = new REDQueue();

  //See for details of how those values are calucated below
  //[1] Sally Floyd and Van Jacobson, "Random Early Detection Gateways with Congestion Avoidance",
  //    IEEE/ACM Transactions on Networking, vol. 1, n. 4, august 1993.
  //
  //[2] Kostas Pentikousis, "Active Queue Management", ACM Crossroads, vol. 7, n. 5,
  //    mid-summer 2001
  //
  //[3] Stefann De Cnodder, Omar Ecoumi, Kenny Paulwels, "RED behavior with different packet sizes",
  //    5th IEEE Symposium on Computers and Communication, (ISCC 2000)
  //
  //[4] http://www.opalsoft.net/qos/DS-26.htm
  //
  //short explanation:
  // q_weight = fixed to 0.002 in most literature
  // min_bytes = max / 3 = 16,666,666
  // max_bytes = mean_bw * max_tolerable_latency, set to 1e8 * 0.5 = 50,000,000
  // limit_bytes = 8 * max = 400,000,000
  // prob = follow most literature 0.02
  // avgpkt = fixed to the same TCP segment size, 1000 Bytes
  //
  // burst = (2*(min+max))/(3*avgpkt) ***DON'T USED BY GTNetS***
  REDQueue *default_red_queue_ = new REDQueue(0.002, 16666666, 50000000, 400000000, 0.02, 1000);

  Queue::Default(*default_red_queue_);
  delete default_red_queue_;

  TCP::DefaultAdvWin(wsize);
  TCP::DefaultSegSize(1000);
  TCP::DefaultTxBuffer(128000);
  TCP::DefaultRxBuffer(128000);

  sim_ = new Simulator();
  sim_->verbose=false;
  topo_ = new GTNETS_Topology();

  // Manual routing
  rm_ = new RoutingManual();
  Routing::SetRouting(rm_);
}

// Constructor.
// TODO: check the default values.
GTSim::GTSim(){
  int wsize = 20000;
  is_topology_ = 0;
  nflow_ = 0;
  jitter_ = 0;
  jitter_seed_ = 10;

  // EXTRACTED FROM GTNETS SOURCE CODE COMMENTS
  //  REDQueue::REDQueue(
  //     DCount_t in_w_q, Count_t in_min_th, Count_t in_max_th,
  //     Count_t in_limit, DCount_t in_max_p, Count_t in_mean_pktsize) : iface(nil)
  // Set default values.
  //Doc:Desc This constructor the critical RED parameters and builds a
  //Doc:Desc correspoding RED queue
  //Doc:Arg1 weight of the queue
  //Doc:Arg2 minimum threshold
  //Doc:Arg3 maximum threshold
  //Doc:Arg4 Limit/max size for the queue
  //Doc:Arg5 maximum value for mark/drop probability
  //Doc:Arg6 Average packet size

  //Default Parameters
  //REDQueue *default_red_queue_ = new REDQueue(0.002, 2500, 7500, 30000, 0.10, 500);
  //Same as above
  //REDQueue *default_red_queue_ = new REDQueue();

  //See for details of how those values are calucated below
  //[1] Sally Floyd and Van Jacobson, "Random Early Detection Gateways with Congestion Avoidance",
  //    IEEE/ACM Transactions on Networking, vol. 1, n. 4, august 1993.
  //
  //[2] Kostas Pentikousis, "Active Queue Management", ACM Crossroads, vol. 7, n. 5,
  //    mid-summer 2001
  //
  //[3] Stefann De Cnodder, Omar Ecoumi, Kenny Paulwels, "RED behavior with different packet sizes",
  //    5th IEEE Symposium on Computers and Communication, (ISCC 2000)
  //
  //[4] http://www.opalsoft.net/qos/DS-26.htm
  //
  //short explanation:
  // q_weight = fixed to 0.002 in most literature
  // min_bytes = max / 3 = 16,666,666
  // max_bytes = mean_bw * max_tolerable_latency, set to 1e8 * 0.5 = 50,000,000
  // limit_bytes = 8 * max = 400,000,000
  // prob = follow most literature 0.02
  // avgpkt = fixed to the same TCP segment size, 1000 Bytes
  //
  // burst = (2*(min+max))/(3*avgpkt) ***DON'T USED BY GTNetS***
  REDQueue *default_red_queue_ = new REDQueue(0.002, 16666666, 50000000, 400000000, 0.02, 1000);

  Queue::Default(*default_red_queue_);
  delete default_red_queue_;

  TCP::DefaultAdvWin(wsize);
  TCP::DefaultSegSize(1000);
  TCP::DefaultTxBuffer(128000);
  TCP::DefaultRxBuffer(128000);

  sim_ = new Simulator();
  sim_->verbose=false;
  topo_ = new GTNETS_Topology();

  // Manual routing
  rm_ = new RoutingManual();
  Routing::SetRouting(rm_);
}

GTSim::~GTSim(){

  map<int, Linkp2p*>::iterator it;
  for (it = gtnets_links_.begin(); it != gtnets_links_.end(); it++){
    delete it->second;
  }
  while (!gtnets_links_.empty())
    gtnets_links_.erase(gtnets_links_.begin());

  map<int, Uniform*>::iterator it2;
  for (it2 = uniform_jitter_generator_.begin(); it2 != uniform_jitter_generator_.end(); it2++){
    delete it2->second;
  }
  while (!uniform_jitter_generator_.empty())
    uniform_jitter_generator_.erase(uniform_jitter_generator_.begin());

  map<int, Node*>::iterator it3;
  for (it3 = gtnets_nodes_.begin(); it3 != gtnets_nodes_.end(); it3++){
    delete it3->second;
  }
  while (!gtnets_nodes_.empty())
    gtnets_nodes_.erase(gtnets_nodes_.begin());

  map<int, TCPServer*>::iterator it4;
  for (it4 = gtnets_servers_.begin(); it4 != gtnets_servers_.end(); it4++){
    delete it4->second;
  }
  while (!gtnets_servers_.empty())
    gtnets_servers_.erase(gtnets_servers_.begin());

  map<int, TCPSend*>::iterator it5;
  for (it5 = gtnets_clients_.begin(); it5 != gtnets_clients_.end(); it5++){
    delete it5->second;
  }
  while (!gtnets_clients_.empty())
    gtnets_clients_.erase(gtnets_clients_.begin());

  is_topology_ = 0;
  delete sim_;
  delete topo_;
  delete rm_;
  sim_ = 0;
  topo_ = 0;
  rm_ = 0;
}

int GTSim::add_router(int id){
  xbt_assert(!(topo_->add_router(id) < 0), "can't add router %d. already exists", id);
}

//bandwidth: in bytes.
//latency: in seconds.
int GTSim::add_link(int id, double bandwidth, double latency){
  double bw = bandwidth * 8; //Bandwidth in bits (used in GTNETS).
  xbt_assert(!(topo_->add_link(id) < 0),"Can't add link %d. already exists", id);
  XBT_DEBUG("Creating a new P2P, linkid %d, bandwidth %gl, latency %gl", id, bandwidth, latency);
  gtnets_links_[id] = new Linkp2p(bw, latency);
  if(jitter_ > 0){
	XBT_DEBUG("Using jitter %f, and seed %u", jitter_, jitter_seed_);
	double min = -1*jitter_*latency;
	double max = jitter_*latency;
	uniform_jitter_generator_[id] = new Uniform(min,max);
	gtnets_links_[id]->Jitter((const Random &) *(uniform_jitter_generator_[id]));
  }
  return 0;
}

// if gtnets_nodes_ includes id, return true, otherwise return false.
bool GTSim::node_include(int id){
  if (gtnets_nodes_.find(id) != gtnets_nodes_.end()) return true;
  else return false;
}

// if gtnets_link_ includes id, return true, otherwise return false.
bool GTSim::link_include(int id){
  if (gtnets_links_.find(id) != gtnets_links_.end()) return true;
  else return false;
}

int GTSim::add_onehop_route(int src, int dst, int link){
  xbt_assert(!(topo_->add_onehop_route(src, dst, link) < 0), "Cannot add a route, src: %d, dst: %d, link: %d", src, dst, link);
  return 0;
}

// Generate the gtnets nodes according to topo_.
void GTSim::add_nodes(){
  static unsigned int address = IPAddr("192.168.0.1");
  IPAddr helper = IPAddr();
  vector<GTNETS_Node*> nodes = topo_->nodes();
  vector<GTNETS_Node*>::iterator it;
  int id;
  for (it = nodes.begin(); it != nodes.end(); it++){
    id = (*it)->id();
    gtnets_nodes_[id] = new Node();
    gtnets_nodes_[id]->SetIPAddr(address++);
    XBT_DEBUG("In GTSim, add_node: %d, with IPAddr %s", id, helper.ToDotted(address-1));

  }
}

void GTSim::node_connect(){

  map<int, GTNETS_Link*> links = topo_->links();
  map<int, GTNETS_Link*>::iterator it;
  int linkid, srcid, dstid;
  for (it = links.begin(); it != links.end(); it++){
    linkid = it->second->id();
    //if link is used in a route, connect the two nodes.
    if (it->second->src_node() && it->second->dst_node()){

      srcid = it->second->src_node()->id();
      dstid = it->second->dst_node()->id();

      gtnets_nodes_[srcid]->
	AddDuplexLink(gtnets_nodes_[dstid], *(gtnets_links_[linkid]));
    XBT_DEBUG("Setting DuplexLink, src %d, dst %d, linkid %d", srcid, dstid, linkid);
    }
  }
}

// Create nodes and routes from the temporary topology, GTNETS_Topolgy.
void GTSim::create_gtnets_topology(){
  add_nodes();
  node_connect();
}

void GTSim::print_topology(){
  topo_->print_topology();
}

// Add a route that includes more than one hop. All one hop
// routes must have been added. When this function is called
// for the first time, all gtnets nodes are generated.
int GTSim::add_route(int src, int dst, int* links, int nlink){
  if (is_topology_ == 0){
    create_gtnets_topology();
    is_topology_ = 1;
  }  

  IPAddr_t mymask = IPAddr("255.255.255.255");

  int src_node = topo_->nodeid_from_hostid(src);
  int dst_node = topo_->nodeid_from_hostid(dst);

  xbt_assert(!(gtnets_nodes_.find(src_node) == gtnets_nodes_.end()), "Node %d not found", src_node);
  xbt_assert(!(gtnets_nodes_.find(dst_node) == gtnets_nodes_.end()), "Node %d not found", dst_node);

  Node* tmpsrc = gtnets_nodes_[src_node];
  Node* tmpdst = gtnets_nodes_[dst_node];

  int next_node, cur_node;
  
  cur_node = src_node;
  for (int i = 0; i < nlink; i++){
	xbt_assert(!(gtnets_nodes_.find(cur_node) == gtnets_nodes_.end()), "Node %d not found", cur_node);
    next_node = topo_->peer_node_id(links[i], cur_node);
    xbt_assert(!(next_node < 0), "Peer node not found");
    xbt_assert(!(gtnets_nodes_.find(next_node) == gtnets_nodes_.end()), "Node %d not found", next_node);
    
    //add route
    Node* tmpcur = gtnets_nodes_[cur_node];
    Node* tmpnext = gtnets_nodes_[next_node];

    tmpcur->AddRoute(tmpdst->GetIPAddr(),
		     mymask,
		     tmpcur->GetIfByNode(tmpnext),
		     tmpnext->GetIPAddr());

    tmpnext->AddRoute(tmpsrc->GetIPAddr(),
		      mymask,
		      tmpnext->GetIfByNode(tmpcur),
		      tmpcur->GetIPAddr());
    
    cur_node = next_node;
  }

  xbt_assert(!(cur_node != dst_node), "Route inconsistency, last: %d, dst: %d",cur_node, dst_node);

  return 0;
}



int GTSim::create_flow(int src, int dst, long datasize, void* metadata){
  //if no route with more than one links, topology has not been generated.
  //generate it here.
  if (is_topology_ == 0){
    create_gtnets_topology();
    is_topology_ = 1;
  }

  int src_node = topo_->nodeid_from_hostid(src);
  xbt_assert(!(src_node < 0), "Src %d not found", src_node);

  int dst_node = topo_->nodeid_from_hostid(dst);
  xbt_assert(!(dst_node < 0), "Dst %d not found", dst_node);

  gtnets_servers_[nflow_] = (TCPServer*) gtnets_nodes_[dst_node]->
       AddApplication(TCPServer(TCPReno()));
  gtnets_servers_[nflow_]->BindAndListen(1000+nflow_);

  gtnets_clients_[nflow_] = (TCPSend*)gtnets_nodes_[src_node]->
    AddApplication(TCPSend(metadata, gtnets_nodes_[dst_node]->GetIPAddr(), 
			   1000+nflow_, Constant(datasize), TCPReno()));
  gtnets_clients_[nflow_]->SetSendCallBack(tcp_sent_callback);
  gtnets_clients_[nflow_]->Start(0);

  gtnets_action_to_flow_[metadata] = nflow_;
  nflow_++;

  return 0;
}

Time_t GTSim::get_time_to_next_flow_completion(){
  int status;
  Time_t t1;
  int pfds[2];
  int soon_pid=-1;
  meta_flg=0;

  //remain needs to be updated in the future
  Count_t remain;
  
  pipe(pfds);
  
  t1 = 0;
  fflush (NULL);
  if( (soon_pid=fork()) != 0){
    read(pfds[0], &t1, sizeof(Time_t));
    waitpid(soon_pid, &status, 0);      
  }else{
    Time_t t;
    t = sim_->RunUntilNextCompletion();
    write(pfds[1], (const void*)&t, sizeof(Time_t));
    exit(0);
  }

  return t1;
}

double GTSim::gtnets_get_flow_rx(void *metadata){
  int flow_id = gtnets_action_to_flow_[metadata];
  return gtnets_servers_[flow_id]->GetTotRx(); 
}


int GTSim::run_until_next_flow_completion(void ***metadata, int *number_of_flows){

  meta_flows.clear();
  meta_nflow = number_of_flows;
  meta_flg = 1;

  Time_t t1 = sim_->RunUntilNextCompletion();

  *metadata = (meta_flows.empty() ? NULL : &meta_flows[0]);
  return 0;
}

int GTSim::run(double delta){
  meta_flg=0;
  sim_->Run(delta);
  return 0;
}

void GTSim::set_jitter(double d){
  xbt_assert(((0 <= d)&&(d <= 1)), "The jitter value must be within interval [0.0;1.0), got %f", d);
  jitter_ = d;
}

void GTSim::set_jitter_seed(int s){
  jitter_seed_ = s;

  if(jitter_seed_ > 0.0){
    XBT_INFO("Setting the jitter_seed with %d", jitter_seed_ );
    Random::GlobalSeed(jitter_seed_  , jitter_seed_  , jitter_seed_  ,jitter_seed_  ,jitter_seed_  ,jitter_seed_);
  }
}

void static tcp_sent_callback(void* action, double completion_time){
  // Schedule the flow complete event.
  SimulatorEvent* e =
    new SimulatorEvent(SimulatorEvent::FLOW_COMPLETE);
  Simulator::instance->Schedule(e, 0, Simulator::instance);

  if (meta_flg){
    meta_flows.push_back(action);
    (*meta_nflow)++;
  }
}


