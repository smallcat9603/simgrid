/* Copyright (c) 2006-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package bittorrent;
/**
 * Common constants for use in the simulation
 */
public class Common {	
		
	public static String TRACKER_MAILBOX = "tracker_mailbox";
	
	public static int FILE_SIZE = 5120;
	public static int FILE_PIECE_SIZE = 512;
	public static int FILE_PIECES = 10;
	public static int PIECES_BLOCKS = 5;
	
	public static int BLOCKS_REQUESTED = 2;
	
	public static int PIECE_COMM_SIZE = 1;
	/**
	 * Information message size
	 */
	public static int MESSAGE_SIZE = 1;
	/**
	 * Max number of pairs sent by the tracker to clients
	 */
	public static int MAXIMUM_PEERS = 50;
	/**
	 * Interval of time where the peer should send a request to the tracker
	 */
	public static int TRACKER_QUERY_INTERVAL = 1000;
	/**
	 * Communication size for a task to the tracker
	 */
	public static double TRACKER_COMM_SIZE = 0.01;
	/**
	 * Timeout for the get peers data
	 */
	public static int GET_PEERS_TIMEOUT = 10000;
	/**
	 * Timeout for "standard" messages.
	 */
	public static int TIMEOUT_MESSAGE = 10;
	/**
	 * Timeout for tracker receive.
	 */
	public static int TRACKER_RECEIVE_TIMEOUT = 10;
	/**
	 * Number of peers that can be unchocked at a given time
	 */
	public static int MAX_UNCHOKED_PEERS = 4;
	/**
	 * Interval between each update of the choked peers
	 */
	public static int UPDATE_CHOKED_INTERVAL = 30;
	/**
	 * Number of pieces the peer asks for simultaneously
	 */
	public static int MAX_PIECES = 1;
}
