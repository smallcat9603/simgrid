/* Copyright (c) 2006-2020. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

package dht.chord;

public class NotifyTask extends ChordTask {
  private int requestId;

  public NotifyTask(String issuerHostname, String answerTo, int requestId) {
    super(issuerHostname, answerTo);
    this.requestId = requestId;
  }

  public int getRequestId(){
    return this.requestId;
  }
}
