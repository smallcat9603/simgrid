/* Copyright (c) 2012-2018. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef BITTORRENT_PEER_HPP
#define BITTORRENT_PEER_HPP
#include "s4u-bittorrent.hpp"
#include <set>
#include <unordered_map>

class Connection {
public:
  int id; // Peer id
  simgrid::s4u::MailboxPtr mailbox_;
  unsigned int bitfield = 0U; // Fields
  //  int messages_count;
  double peer_speed    = 0;
  double last_unchoke  = 0;
  int current_piece    = -1;
  bool am_interested   = false; // Indicates if we are interested in something the peer has
  bool interested      = false; // Indicates if the peer is interested in one of our pieces
  bool choked_upload   = true;  // Indicates if the peer is choked for the current peer
  bool choked_download = true;  // Indicates if the peer has choked the current peer

  explicit Connection(int id) : id(id), mailbox_(simgrid::s4u::Mailbox::by_name(std::to_string(id))){};
  ~Connection() = default;
  void addSpeedValue(double speed) { peer_speed = peer_speed * 0.6 + speed * 0.4; }
  bool hasPiece(unsigned int piece) { return bitfield & 1U << piece; }
};

class Peer {
  int id;
  double deadline;
  RngStream stream;
  simgrid::s4u::MailboxPtr mailbox_;
  std::unordered_map<int, Connection*> connected_peers;
  std::set<Connection*> active_peers; // active peers list

  unsigned int bitfield_             = 0;       // list of pieces the peer has.
  unsigned long long bitfield_blocks = 0;       // list of blocks the peer has.
  short* pieces_count                = nullptr; // number of peers that have each piece.
  unsigned int current_pieces        = 0;       // current pieces the peer is downloading
  double begin_receive_time = 0; // time when the receiving communication has begun, useful for calculating host speed.
  int round_                = 0; // current round for the chocking algorithm.

  simgrid::s4u::CommPtr comm_received = nullptr; // current comm
  Message* message                    = nullptr; // current message being received
public:
  explicit Peer(std::vector<std::string> args);
  Peer(const Peer&) = delete;
  Peer& operator=(const Peer&) = delete;
  ~Peer();
  void operator()();

  std::string getStatus();
  bool hasFinished();
  int nbInterestedPeers();
  bool isInterestedBy(Connection* remote_peer);
  bool isInterestedByFree(Connection* remote_peer);
  void updateActivePeersSet(Connection* remote_peer);
  void updateInterestedAfterReceive();
  void updateChokedPeers();

  bool hasNotPiece(unsigned int piece) { return not(bitfield_ & 1U << piece); }
  bool hasCompletedPiece(unsigned int piece);
  unsigned int countPieces(unsigned int bitfield);
  /** Check that a piece is not currently being download by the peer. */
  bool isNotDownloadingPiece(unsigned int piece) { return not(current_pieces & 1U << piece); }
  int partiallyDownloadedPiece(Connection* remote_peer);
  void updatePiecesCountFromBitfield(unsigned int bitfield);
  void removeCurrentPiece(Connection* remote_peer, unsigned int current_piece);
  void updateBitfieldBlocks(int piece, int block_index, int block_length);
  int getFirstMissingBlockFrom(int piece);
  int selectPieceToDownload(Connection* remote_peer);
  void requestNewPieceTo(Connection* remote_peer);

  bool getPeersFromTracker();
  void sendMessage(simgrid::s4u::MailboxPtr mailbox, e_message_type type, uint64_t size);
  void sendBitfield(simgrid::s4u::MailboxPtr mailbox);
  void sendPiece(simgrid::s4u::MailboxPtr mailbox, unsigned int piece, int block_index, int block_length);
  void sendHandshakeToAllPeers();
  void sendHaveToAllPeers(unsigned int piece);
  void sendRequestTo(Connection* remote_peer, unsigned int piece);

  void handleMessage();
  void leech();
  void seed();
};

#endif /* BITTORRENT_PEER_HPP */
