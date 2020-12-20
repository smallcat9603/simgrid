/* Copyright (c) 2012-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package dht.kademlia;

public class Contact implements Comparable<Object> {
  private int id;
  private int distance;

  public Contact(int id, int distance) {
    this.id = id;
    this.distance = distance;
  }

  public int getId() {
    return id;
  }

  public int getDistance() {
    return distance;
  }

  @Override
  public boolean equals(Object x) {
    return x != null && x.equals(id);
  }

  @Override
  public int hashCode() {
    int hash = 1;
    hash = hash * 17 + id;
    hash = hash * 31 + distance;
    return hash;
  }

  @Override
  public int compareTo(Object o) {
    Contact c = (Contact)o;
    if (distance < c.distance) {
      return -1;
    }
    else if (distance == c.distance) {
      return 0;
    }
    else {
      return 1;
    }
  }

  @Override
  public String toString() {
    return "Contact [id=" + id + ", distance=" + distance + "]";
  }

}
