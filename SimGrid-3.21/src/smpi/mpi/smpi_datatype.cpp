/* smpi_datatype.cpp -- MPI primitives to handle datatypes                  */
/* Copyright (c) 2009-2018. The SimGrid Team.  All rights reserved.         */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "private.hpp"
#include "simgrid/modelchecker.h"
#include "smpi_datatype_derived.hpp"
#include "smpi_op.hpp"
#include "src/smpi/include/smpi_actor.hpp"

#include <string>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(smpi_datatype, smpi, "Logging specific to SMPI (datatype)");

static std::unordered_map<std::string, simgrid::smpi::Datatype*> id2type_lookup;

#define CREATE_MPI_DATATYPE(name, id, type)                                                                            \
  static simgrid::smpi::Datatype mpi_##name((char*)#name, id, sizeof(type), /* size */                                 \
                                            0,                              /* lb */                                   \
                                            sizeof(type),                   /* ub = lb + size */                       \
                                            DT_FLAG_BASIC                   /* flags */                                \
                                            );                                                                         \
  const MPI_Datatype name = &mpi_##name;

#define CREATE_MPI_DATATYPE_NULL(name, id)                                                                             \
  static simgrid::smpi::Datatype mpi_##name((char*)#name, id, 0, /* size */                                            \
                                            0,                   /* lb */                                              \
                                            0,                   /* ub = lb + size */                                  \
                                            DT_FLAG_BASIC        /* flags */                                           \
                                            );                                                                         \
  const MPI_Datatype name = &mpi_##name;

// Predefined data types
CREATE_MPI_DATATYPE(MPI_CHAR, 2, char);
CREATE_MPI_DATATYPE(MPI_SHORT, 3, short);
CREATE_MPI_DATATYPE(MPI_INT, 1, int);
CREATE_MPI_DATATYPE(MPI_LONG, 4, long);
CREATE_MPI_DATATYPE(MPI_LONG_LONG, 7, long long);
CREATE_MPI_DATATYPE(MPI_SIGNED_CHAR, 8, signed char);
CREATE_MPI_DATATYPE(MPI_UNSIGNED_CHAR, 9, unsigned char);
CREATE_MPI_DATATYPE(MPI_UNSIGNED_SHORT, 10, unsigned short);
CREATE_MPI_DATATYPE(MPI_UNSIGNED, 11, unsigned int);
CREATE_MPI_DATATYPE(MPI_UNSIGNED_LONG, 12, unsigned long);
CREATE_MPI_DATATYPE(MPI_UNSIGNED_LONG_LONG, 13, unsigned long long);
CREATE_MPI_DATATYPE(MPI_FLOAT, 5, float);
CREATE_MPI_DATATYPE(MPI_DOUBLE, 0, double);
CREATE_MPI_DATATYPE(MPI_LONG_DOUBLE, 14, long double);
CREATE_MPI_DATATYPE(MPI_WCHAR, 15, wchar_t);
CREATE_MPI_DATATYPE(MPI_C_BOOL, 16, bool);
CREATE_MPI_DATATYPE(MPI_BYTE, 6, int8_t);
CREATE_MPI_DATATYPE(MPI_INT8_T, 17, int8_t);
CREATE_MPI_DATATYPE(MPI_INT16_T, 18, int16_t);
CREATE_MPI_DATATYPE(MPI_INT32_T, 19, int32_t);
CREATE_MPI_DATATYPE(MPI_INT64_T, 20, int64_t);
CREATE_MPI_DATATYPE(MPI_UINT8_T, 21, uint8_t);
CREATE_MPI_DATATYPE(MPI_UINT16_T, 22, uint16_t);
CREATE_MPI_DATATYPE(MPI_UINT32_T, 23, uint32_t);
CREATE_MPI_DATATYPE(MPI_UINT64_T, 24, uint64_t);
CREATE_MPI_DATATYPE(MPI_C_FLOAT_COMPLEX, 25, float _Complex);
CREATE_MPI_DATATYPE(MPI_C_DOUBLE_COMPLEX, 26, double _Complex);
CREATE_MPI_DATATYPE(MPI_C_LONG_DOUBLE_COMPLEX, 27, long double _Complex);
CREATE_MPI_DATATYPE(MPI_AINT, 28, MPI_Aint);
CREATE_MPI_DATATYPE(MPI_OFFSET, 29, MPI_Offset);

CREATE_MPI_DATATYPE(MPI_FLOAT_INT, 30, float_int);
CREATE_MPI_DATATYPE(MPI_LONG_INT, 31, long_int);
CREATE_MPI_DATATYPE(MPI_DOUBLE_INT, 32, double_int);
CREATE_MPI_DATATYPE(MPI_SHORT_INT, 33, short_int);
CREATE_MPI_DATATYPE(MPI_2INT, 34, int_int);
CREATE_MPI_DATATYPE(MPI_2FLOAT, 35, float_float);
CREATE_MPI_DATATYPE(MPI_2DOUBLE, 36, double_double);
CREATE_MPI_DATATYPE(MPI_2LONG, 37, long_long);

CREATE_MPI_DATATYPE(MPI_REAL, 38, float);
CREATE_MPI_DATATYPE(MPI_REAL4, 39, float);
CREATE_MPI_DATATYPE(MPI_REAL8, 40, double);
CREATE_MPI_DATATYPE(MPI_REAL16, 41, long double);
CREATE_MPI_DATATYPE_NULL(MPI_DATATYPE_NULL, -1);
CREATE_MPI_DATATYPE_NULL(MPI_COMPLEX8, 42);
CREATE_MPI_DATATYPE_NULL(MPI_COMPLEX16, 43);
CREATE_MPI_DATATYPE_NULL(MPI_COMPLEX32, 44);
CREATE_MPI_DATATYPE(MPI_INTEGER1, 45, int);
CREATE_MPI_DATATYPE(MPI_INTEGER2, 46, int16_t);
CREATE_MPI_DATATYPE(MPI_INTEGER4, 47, int32_t);
CREATE_MPI_DATATYPE(MPI_INTEGER8, 48, int64_t);
CREATE_MPI_DATATYPE(MPI_INTEGER16, 49, integer128_t);

CREATE_MPI_DATATYPE(MPI_LONG_DOUBLE_INT, 50, long_double_int);

CREATE_MPI_DATATYPE_NULL(MPI_UB, 51);
CREATE_MPI_DATATYPE_NULL(MPI_LB, 52);
CREATE_MPI_DATATYPE(MPI_PACKED, 53, char);
// Internal use only
CREATE_MPI_DATATYPE(MPI_PTR, 54, void*);

namespace simgrid{
namespace smpi{

std::unordered_map<int, smpi_key_elem> Datatype::keyvals_; // required by the Keyval class implementation
int Datatype::keyval_id_=0; // required by the Keyval class implementation
Datatype::Datatype(int ident, int size, MPI_Aint lb, MPI_Aint ub, int flags) : Datatype(size, lb, ub, flags)
{
  id = std::to_string(ident);
}
Datatype::Datatype(int size,MPI_Aint lb, MPI_Aint ub, int flags) : name_(nullptr), size_(size), lb_(lb), ub_(ub), flags_(flags), refcount_(1){
#if SIMGRID_HAVE_MC
  if(MC_is_active())
    MC_ignore(&(refcount_), sizeof(refcount_));
#endif
}

//for predefined types, so in_use = 0.
Datatype::Datatype(char* name, int ident, int size, MPI_Aint lb, MPI_Aint ub, int flags)
    : name_(name), id(std::to_string(ident)), size_(size), lb_(lb), ub_(ub), flags_(flags), refcount_(0)
{
  id2type_lookup.insert({id, this});
#if SIMGRID_HAVE_MC
  if(MC_is_active())
    MC_ignore(&(refcount_), sizeof(refcount_));
#endif
}

Datatype::Datatype(Datatype *datatype, int* ret) : name_(nullptr), lb_(datatype->lb_), ub_(datatype->ub_), flags_(datatype->flags_), refcount_(1)
{
  flags_ &= ~DT_FLAG_PREDEFINED;
  *ret = MPI_SUCCESS;
  if(datatype->name_)
    name_ = xbt_strdup(datatype->name_);
    
  if (not datatype->attributes()->empty()) {
    int flag=0;
    void* value_out;
    for (auto const& it : *(datatype->attributes())) {
      smpi_key_elem elem = keyvals_.at(it.first);
      if (elem != nullptr){
        if( elem->copy_fn.type_copy_fn != MPI_NULL_COPY_FN && 
            elem->copy_fn.type_copy_fn != MPI_TYPE_DUP_FN)
          *ret = elem->copy_fn.type_copy_fn(datatype, it.first, elem->extra_state, it.second, &value_out, &flag);
        else if ( elem->copy_fn.type_copy_fn_fort != MPI_NULL_COPY_FN &&
                  (*(int*)*elem->copy_fn.type_copy_fn_fort) != 1){
          value_out=(int*)xbt_malloc(sizeof(int));
          elem->copy_fn.type_copy_fn_fort(datatype, it.first, elem->extra_state, it.second, value_out, &flag,ret);
        }
        if (*ret != MPI_SUCCESS) {
          break;
        }
        if(elem->copy_fn.type_copy_fn == MPI_TYPE_DUP_FN || 
          ((elem->copy_fn.type_copy_fn_fort != MPI_NULL_COPY_FN) && (*(int*)*elem->copy_fn.type_copy_fn_fort == 1))){
          elem->refcount++;
          attributes()->insert({it.first, it.second});
        } else if (flag){
          elem->refcount++;
          attributes()->insert({it.first, value_out});
        }
      }
    }
  }
}

Datatype::~Datatype(){
  xbt_assert(refcount_ >= 0);

  if(flags_ & DT_FLAG_PREDEFINED)
    return;

  //if still used, mark for deletion
  if(refcount_!=0){
      flags_ |=DT_FLAG_DESTROYED;
      return;
  }

  cleanup_attr<Datatype>();

  xbt_free(name_);
}


void Datatype::ref(){

  refcount_++;

#if SIMGRID_HAVE_MC
  if(MC_is_active())
    MC_ignore(&(refcount_), sizeof(refcount_));
#endif
}

void Datatype::unref(MPI_Datatype datatype)
{
  if (datatype->refcount_ > 0)
    datatype->refcount_--;

  if (datatype->refcount_ == 0 && not(datatype->flags_ & DT_FLAG_PREDEFINED))
    delete datatype;

#if SIMGRID_HAVE_MC
  if(MC_is_active())
    MC_ignore(&(datatype->refcount_), sizeof(datatype->refcount_));
#endif
}

void Datatype::commit()
{
  flags_ |= DT_FLAG_COMMITED;
}

bool Datatype::is_valid(){
  return (flags_ & DT_FLAG_COMMITED);
}

bool Datatype::is_basic()
{
  return (flags_ & DT_FLAG_BASIC);
}

const char* Datatype::encode(MPI_Datatype dt)
{
  return dt->id.c_str();
}

MPI_Datatype Datatype::decode(std::string datatype_id)
{
  return id2type_lookup.find(datatype_id)->second;
}

bool Datatype::is_replayable()
{
  return ((this==MPI_BYTE)||(this==MPI_DOUBLE)||(this==MPI_INT)||
          (this==MPI_CHAR)||(this==MPI_SHORT)||(this==MPI_LONG)||(this==MPI_FLOAT));
}

size_t Datatype::size(){
  return size_;
}

int Datatype::flags(){
  return flags_;
}

int Datatype::refcount(){
  return refcount_;
}

void Datatype::addflag(int flag){
  flags_ &= flag;
}

MPI_Aint Datatype::lb(){
  return lb_;
}

MPI_Aint Datatype::ub(){
  return ub_;
}

char* Datatype::name(){
  return name_;
}


int Datatype::extent(MPI_Aint * lb, MPI_Aint * extent){
  *lb = lb_;
  *extent = ub_ - lb_;
  return MPI_SUCCESS;
}

MPI_Aint Datatype::get_extent(){
  return ub_ - lb_;
}

void Datatype::get_name(char* name, int* length){
  *length = strlen(name_);
  strncpy(name, name_, *length+1);
}

void Datatype::set_name(char* name){
  if(name_!=nullptr &&  (flags_ & DT_FLAG_PREDEFINED) == 0)
    xbt_free(name_);
  name_ = xbt_strdup(name);
}

int Datatype::pack(void* inbuf, int incount, void* outbuf, int outcount, int* position,MPI_Comm comm){
  if (outcount - *position < incount*static_cast<int>(size_))
    return MPI_ERR_BUFFER;
  Datatype::copy(inbuf, incount, this, static_cast<char*>(outbuf) + *position, outcount, MPI_CHAR);
  *position += incount * size_;
  return MPI_SUCCESS;
}

int Datatype::unpack(void* inbuf, int insize, int* position, void* outbuf, int outcount,MPI_Comm comm){
  if (outcount*static_cast<int>(size_)> insize)
    return MPI_ERR_BUFFER;
  Datatype::copy(static_cast<char*>(inbuf) + *position, insize, MPI_CHAR, outbuf, outcount, this);
  *position += outcount * size_;
  return MPI_SUCCESS;
}


int Datatype::copy(void *sendbuf, int sendcount, MPI_Datatype sendtype,
                       void *recvbuf, int recvcount, MPI_Datatype recvtype){

// FIXME Handle the case of a partial shared malloc.

  if (smpi_privatize_global_variables == SmpiPrivStrategies::MMAP) {
    smpi_switch_data_segment(simgrid::s4u::Actor::self());
  }
  /* First check if we really have something to do */
  if (recvcount > 0 && recvbuf != sendbuf) {
    sendcount *= sendtype->size();
    recvcount *= recvtype->size();
    int count = sendcount < recvcount ? sendcount : recvcount;

    if (not(sendtype->flags() & DT_FLAG_DERIVED) && not(recvtype->flags() & DT_FLAG_DERIVED)) {
      if (not smpi_process()->replaying())
        memcpy(recvbuf, sendbuf, count);
    } else if (not(sendtype->flags() & DT_FLAG_DERIVED)) {
      recvtype->unserialize(sendbuf, recvbuf, count / recvtype->size(), MPI_REPLACE);
    } else if (not(recvtype->flags() & DT_FLAG_DERIVED)) {
      sendtype->serialize(sendbuf, recvbuf, count / sendtype->size());
    }else{

      void * buf_tmp = xbt_malloc(count);

      sendtype->serialize( sendbuf, buf_tmp,count/sendtype->size());
      recvtype->unserialize( buf_tmp, recvbuf,count/recvtype->size(), MPI_REPLACE);

      xbt_free(buf_tmp);
    }
  }

  return sendcount > recvcount ? MPI_ERR_TRUNCATE : MPI_SUCCESS;
}

//Default serialization method : memcpy.
void Datatype::serialize( void* noncontiguous_buf, void *contiguous_buf, int count){
  char* contiguous_buf_char = static_cast<char*>(contiguous_buf);
  char* noncontiguous_buf_char = static_cast<char*>(noncontiguous_buf)+lb_;
  memcpy(contiguous_buf_char, noncontiguous_buf_char, count*size_);

}

void Datatype::unserialize( void* contiguous_buf, void *noncontiguous_buf, int count, MPI_Op op){
  char* contiguous_buf_char = static_cast<char*>(contiguous_buf);
  char* noncontiguous_buf_char = static_cast<char*>(noncontiguous_buf)+lb_;
  int n=count;
  if(op!=MPI_OP_NULL)
    op->apply( contiguous_buf_char, noncontiguous_buf_char, &n, this);
}

int Datatype::create_contiguous(int count, MPI_Datatype old_type, MPI_Aint lb, MPI_Datatype* new_type){
  if(old_type->flags_ & DT_FLAG_DERIVED){
    //handle this case as a hvector with stride equals to the extent of the datatype
    return create_hvector(count, 1, old_type->get_extent(), old_type, new_type);
  }
  if(count>0)
    *new_type = new Type_Contiguous(count * old_type->size(), lb, lb + count * old_type->size(),
                                   DT_FLAG_DERIVED, count, old_type);
  else
    *new_type = new Datatype(count * old_type->size(), lb, lb + count * old_type->size(),0);
  return MPI_SUCCESS;
}

int Datatype::create_vector(int count, int block_length, int stride, MPI_Datatype old_type, MPI_Datatype* new_type)
{
  int retval;
  if (block_length<0)
    return MPI_ERR_ARG;
  MPI_Aint lb = 0;
  MPI_Aint ub = 0;
  if(count>0){
    lb=old_type->lb();
    ub=((count-1)*stride+block_length-1)*old_type->get_extent()+old_type->ub();
  }
  if(old_type->flags() & DT_FLAG_DERIVED || stride != block_length){
    *new_type = new Type_Vector(count * (block_length) * old_type->size(), lb, ub,
                                   DT_FLAG_DERIVED, count, block_length, stride, old_type);
    retval=MPI_SUCCESS;
  }else{
    /* in this situation the data are contiguous thus it's not required to serialize and unserialize it*/
    *new_type = new Datatype(count * block_length * old_type->size(), 0, ((count -1) * stride + block_length)*
                         old_type->size(), DT_FLAG_CONTIGUOUS);
    retval=MPI_SUCCESS;
  }
  return retval;
}


int Datatype::create_hvector(int count, int block_length, MPI_Aint stride, MPI_Datatype old_type, MPI_Datatype* new_type)
{
  int retval;
  if (block_length<0)
    return MPI_ERR_ARG;
  MPI_Aint lb = 0;
  MPI_Aint ub = 0;
  if(count>0){
    lb=old_type->lb();
    ub=((count-1)*stride)+(block_length-1)*old_type->get_extent()+old_type->ub();
  }
  if(old_type->flags() & DT_FLAG_DERIVED || stride != block_length*old_type->get_extent()){
    *new_type = new Type_Hvector(count * (block_length) * old_type->size(), lb, ub,
                                   DT_FLAG_DERIVED, count, block_length, stride, old_type);
    retval=MPI_SUCCESS;
  }else{
    /* in this situation the data are contiguous thus it's not required to serialize and unserialize it*/
    *new_type = new Datatype(count * block_length * old_type->size(), 0, count * block_length * old_type->size(), DT_FLAG_CONTIGUOUS);
    retval=MPI_SUCCESS;
  }
  return retval;
}

int Datatype::create_indexed(int count, int* block_lengths, int* indices, MPI_Datatype old_type, MPI_Datatype* new_type){
  int size = 0;
  bool contiguous=true;
  MPI_Aint lb = 0;
  MPI_Aint ub = 0;
  if(count>0){
    lb=indices[0]*old_type->get_extent();
    ub=indices[0]*old_type->get_extent() + block_lengths[0]*old_type->ub();
  }

  for (int i = 0; i < count; i++) {
    if (block_lengths[i] < 0)
      return MPI_ERR_ARG;
    size += block_lengths[i];

    if(indices[i]*old_type->get_extent()+old_type->lb()<lb)
      lb = indices[i]*old_type->get_extent()+old_type->lb();
    if(indices[i]*old_type->get_extent()+block_lengths[i]*old_type->ub()>ub)
      ub = indices[i]*old_type->get_extent()+block_lengths[i]*old_type->ub();

    if ( (i< count -1) && (indices[i]+block_lengths[i] != indices[i+1]) )
      contiguous=false;
  }
  if(old_type->flags_ & DT_FLAG_DERIVED)
    contiguous=false;

  if (not contiguous) {
    *new_type = new Type_Indexed(size * old_type->size(),lb,ub,
                                 DT_FLAG_DERIVED|DT_FLAG_DATA, count, block_lengths, indices, old_type);
  }else{
    Datatype::create_contiguous(size, old_type, lb, new_type);
  }
  return MPI_SUCCESS;
}

int Datatype::create_hindexed(int count, int* block_lengths, MPI_Aint* indices, MPI_Datatype old_type, MPI_Datatype* new_type){
  int size = 0;
  bool contiguous=true;
  MPI_Aint lb = 0;
  MPI_Aint ub = 0;
  if(count>0){
    lb=indices[0] + old_type->lb();
    ub=indices[0] + block_lengths[0]*old_type->ub();
  }
  for (int i = 0; i < count; i++) {
    if (block_lengths[i] < 0)
      return MPI_ERR_ARG;
    size += block_lengths[i];

    if(indices[i]+old_type->lb()<lb)
      lb = indices[i]+old_type->lb();
    if(indices[i]+block_lengths[i]*old_type->ub()>ub)
      ub = indices[i]+block_lengths[i]*old_type->ub();

    if ( (i< count -1) && (indices[i]+block_lengths[i]*(static_cast<int>(old_type->size())) != indices[i+1]) )
      contiguous=false;
  }
  if (old_type->flags_ & DT_FLAG_DERIVED || lb!=0)
    contiguous=false;

  if (not contiguous) {
    *new_type = new Type_Hindexed(size * old_type->size(),lb,ub,
                                   DT_FLAG_DERIVED|DT_FLAG_DATA, count, block_lengths, indices, old_type);
  }else{
    Datatype::create_contiguous(size, old_type, lb, new_type);
  }
  return MPI_SUCCESS;
}

int Datatype::create_struct(int count, int* block_lengths, MPI_Aint* indices, MPI_Datatype* old_types, MPI_Datatype* new_type){
  size_t size = 0;
  bool contiguous=true;
  size = 0;
  MPI_Aint lb = 0;
  MPI_Aint ub = 0;
  if(count>0){
    lb=indices[0] + old_types[0]->lb();
    ub=indices[0] + block_lengths[0]*old_types[0]->ub();
  }
  bool forced_lb=false;
  bool forced_ub=false;
  for (int i = 0; i < count; i++) {
    if (block_lengths[i]<0)
      return MPI_ERR_ARG;
    if (old_types[i]->flags_ & DT_FLAG_DERIVED)
      contiguous=false;

    size += block_lengths[i]*old_types[i]->size();
    if (old_types[i]==MPI_LB){
      lb=indices[i];
      forced_lb=true;
    }
    if (old_types[i]==MPI_UB){
      ub=indices[i];
      forced_ub=true;
    }

    if (not forced_lb && indices[i] + old_types[i]->lb() < lb)
      lb = indices[i];
    if (not forced_ub && indices[i] + block_lengths[i] * old_types[i]->ub() > ub)
      ub = indices[i]+block_lengths[i]*old_types[i]->ub();

    if ( (i< count -1) && (indices[i]+block_lengths[i]*static_cast<int>(old_types[i]->size()) != indices[i+1]) )
      contiguous=false;
  }
  if (not contiguous) {
    *new_type = new Type_Struct(size, lb,ub, DT_FLAG_DERIVED|DT_FLAG_DATA,
                                count, block_lengths, indices, old_types);
  }else{
    Datatype::create_contiguous(size, MPI_CHAR, lb, new_type);
  }
  return MPI_SUCCESS;
}

int Datatype::create_subarray(int ndims, int* array_of_sizes,
                             int* array_of_subsizes, int* array_of_starts,
                             int order, MPI_Datatype oldtype, MPI_Datatype *newtype){
  MPI_Datatype tmp;

  for (int i = 0; i < ndims; i++) {
    if (array_of_subsizes[i] > array_of_sizes[i]){
      XBT_WARN("subarray : array_of_subsizes > array_of_sizes for dim %d",i);
      return MPI_ERR_ARG;
    }
    if (array_of_starts[i] + array_of_subsizes[i] > array_of_sizes[i]){
      XBT_WARN("subarray : array_of_starts + array_of_subsizes > array_of_sizes for dim %d",i);
      return MPI_ERR_ARG;
    }
  }

  MPI_Aint extent = oldtype->get_extent();

  int i;
  int step;
  int end;
  if( order==MPI_ORDER_C ) {
      i = ndims - 1;
      step = -1;
      end = -1;
  } else {
      i = 0;
      step = 1;
      end = ndims;
  }

  MPI_Aint size = (MPI_Aint)array_of_sizes[i] * (MPI_Aint)array_of_sizes[i+step];
  MPI_Aint lb = (MPI_Aint)array_of_starts[i] + (MPI_Aint)array_of_starts[i+step] *(MPI_Aint)array_of_sizes[i];

  create_vector( array_of_subsizes[i+step], array_of_subsizes[i], array_of_sizes[i],
                               oldtype, newtype );

  tmp = *newtype;

  for( i += 2 * step; i != end; i += step ) {
      create_hvector( array_of_subsizes[i], 1, size * extent,
                                    tmp, newtype );
      unref(tmp);
      lb += size * array_of_starts[i];
      size *= array_of_sizes[i];
      tmp = *newtype;
  }

  MPI_Aint lbs[1] = {lb * extent};
  int sizes [1]={1};
  //handle LB and UB with a resized call
  create_hindexed( 1, sizes, lbs, tmp, newtype);
  unref(tmp);

  tmp = *newtype;
  create_resized(tmp, 0, extent, newtype);

  unref(tmp);
  return MPI_SUCCESS;
}

int Datatype::create_resized(MPI_Datatype oldtype,MPI_Aint lb, MPI_Aint extent, MPI_Datatype *newtype){
  int blocks[3]         = {1, 1, 1};
  MPI_Aint disps[3]     = {lb, 0, lb + extent};
  MPI_Datatype types[3] = {MPI_LB, oldtype, MPI_UB};

  *newtype = new simgrid::smpi::Type_Struct(oldtype->size(), lb, lb + extent, DT_FLAG_DERIVED, 3, blocks, disps, types);

  (*newtype)->addflag(~DT_FLAG_COMMITED);
  return MPI_SUCCESS;
}

Datatype* Datatype::f2c(int id){
  return static_cast<Datatype*>(F2C::f2c(id));
}


}
}

