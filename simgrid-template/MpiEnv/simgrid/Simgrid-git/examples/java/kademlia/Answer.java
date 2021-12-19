/* Copyright (c) 2012-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package kademlia;

import java.util.ArrayList;
import java.util.Collections;

/**
 * Answer to a "FIND_NODE" query. Contains the nodes closest to
 * an id given.
 */
public class Answer {
	/**
	 * Id of the node we're trying to find
	 */
	private int destinationId;
	/**
	 * Closest nodes in the answer.
	 */
	private ArrayList<Contact> nodes;
	
	/**
	 * Constructor
	 */
	public Answer(int destinationId) {
		this.destinationId = destinationId;
		nodes = new ArrayList<Contact>();
	}
	/**
	 * Returns the destination id
	 */
	int getDestinationId() {
		return destinationId;
	}
	/**
	 * Returns the list of the nodes in the answer
	 */
	ArrayList<Contact> getNodes() {
		return nodes;
	}
	/**
	 * Returns the answer array size
	 */
	int size() {
		return nodes.size();
	}
	/**
	 * Remove an element from the answer.
	 */
	public void remove(int index) {
		nodes.remove(index);
	}
	/**
	 * Add a contact to the answer.
	 */
	public void add(Contact contact) {
		nodes.add(contact);
	}
	/**
	 * Merge the contents of this answer with another answer
	 */
	public int merge(Answer answer) {
		int nbAdded = 0;
		
		for (Contact c: answer.getNodes()) {
			if (!nodes.contains(c)) {
				nbAdded++;
				nodes.add(c);
			}
		}
		Collections.sort(nodes);
		//Trim the list
		while (answer.size() > Common.BUCKET_SIZE) {
			answer.remove(answer.size() - 1);
		}
		return nbAdded;
	}
	/**
	 * Returns if the destination has been found
	 */
	public boolean destinationFound() {
		if (nodes.size() < 1) {
			return false;
		}
		Contact tail = nodes.get(0);
		return tail.getDistance() == 0;
	}
	@Override
	public String toString() {
		return "Answer [destinationId=" + destinationId + ", nodes=" + nodes
				+ "]";
	}
	
}
