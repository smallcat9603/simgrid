/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.simgrid.surf;

/**
 * A generic resource for the network component
 */
public class NetworkLink extends Resource {
  private long swigCPtr;

  protected NetworkLink(long cPtr, boolean cMemoryOwn) {
    super(SurfJNI.NetworkLink_SWIGUpcast(cPtr), cMemoryOwn);
    swigCPtr = cPtr;
  }

  protected static long getCPtr(NetworkLink obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        SurfJNI.delete_NetworkLink(swigCPtr);
      }
      swigCPtr = 0;
    }
    super.delete();
  }

  protected static long[] cArrayUnwrap(NetworkLink[] arrayWrapper) {
      long[] cArray = new long[arrayWrapper.length];
      for (int i=0; i<arrayWrapper.length; i++)
        cArray[i] = NetworkLink.getCPtr(arrayWrapper[i]);
      return cArray;
  }

  protected static NetworkLink[] cArrayWrap(long[] cArray, boolean cMemoryOwn) {
    NetworkLink[] arrayWrapper = new NetworkLink[cArray.length];
    for (int i=0; i<cArray.length; i++)
      arrayWrapper[i] = new NetworkLink(cArray[i], cMemoryOwn);
    return arrayWrapper;
  }

  
  /**
    * @return The bandwidth of the link
    */
  public double getBandwidth() {
    return SurfJNI.NetworkLink_getBandwidth(swigCPtr, this);
  }

  
  /**
    * @param value The new bandwidth
    */
  public void updateBandwidth(double value, double date) {
    SurfJNI.NetworkLink_updateBandwidth__SWIG_0(swigCPtr, this, value, date);
  }

  
  /**
    * @param value The new bandwidth
    */
  public void updateBandwidth(double value) {
    SurfJNI.NetworkLink_updateBandwidth__SWIG_1(swigCPtr, this, value);
  }

  
  /**
    * @return The latency of the link
    */
  public double getLatency() {
    return SurfJNI.NetworkLink_getLatency(swigCPtr, this);
  }

  
  /**
    * @param value The new latency
    */
  public void updateLatency(double value, double date) {
    SurfJNI.NetworkLink_updateLatency__SWIG_0(swigCPtr, this, value, date);
  }

  
  /**
    * @param value The new latency
    */
  public void updateLatency(double value) {
    SurfJNI.NetworkLink_updateLatency__SWIG_1(swigCPtr, this, value);
  }

}
