/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package org.simgrid.surf;

public class LmmConstraint {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected LmmConstraint(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(LmmConstraint obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public synchronized void delete() {
    if (swigCPtr != 0) {
      if (swigCMemOwn) {
        swigCMemOwn = false;
        SurfJNI.delete_LmmConstraint(swigCPtr);
      }
      swigCPtr = 0;
    }
  }

  public double getUsage() {
    return SurfJNI.LmmConstraint_getUsage(swigCPtr, this);
  }

  public LmmConstraint() {
    this(SurfJNI.new_LmmConstraint(), true);
  }

}
