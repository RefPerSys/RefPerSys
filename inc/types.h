/*
 * File: refpersys/inc/types.h
 *
 * Description:
 *      This file is part of the Reflective Persistent System. It declares the
 *      refpersys types and their associated constants.
 *
 * Author(s):
 *      Basile Starynkevitch <basile@starynkevitch.net>
 *      Niklas Rosencrantz <niklasro@gmail.com>
 *      Abhishek Chakravarti <abhishek@taranjali.org>
 *
 * Copyright:
 *      (c) 2019 The Reflective Persistent System Team
 *      <https://refpersys.gitlab.io>
 *
 * License:
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/* create header guard */
#if (!defined __REFPERSYS_TYPES_DEFINED)
#       define __REFPERSYS_TYPES_DEFINED


/* include required header files */
#include <stdint.h>
#include "util.h"


/* see https://en.wikipedia.org/wiki/Flexible_array_member
   wikipage. C++ don't really have them, we are mimicking them as the
   last member of some struct which is an array of dimension 0 */
#define RPS_FLEXIBLE_DIM 0

/* open support for C++ */
#if (defined __cplusplus)
extern "C" {
#endif


  /****************************************************************************
   * Section: Object ID Serial (rps_serial63)
   ****************************************************************************/


  /* declare the rps_serial63 type; this is analogous to the Bismon
   * serial64_tyBM type in the header file id_BM.h */
  typedef uint64_t rps_serial63;


  /* number of digits in an object ID serial when represented the form of
   * base 62 digits */
#define RPS_SERIAL63_DIGITS 11


  /* object ID serials are represented compactly in base 62 */
#define RPS_SERIAL63_BASE 62


  /*
   *      RPS_SERIAL63_MIN - minimum object ID serial value
   *
   *      The RPS_SERIAL63_MIN symbolic constant defines the minimum value of an
   *      object ID serial.
   *
   *      TODO: explain why it is 62 * 62
   */
#define RPS_SERIAL63_MIN ((uint64_t) 3884)


  /*
   *      RPS_SERIAL63_MAX - maximum object ID serial value
   *
   *      The RPS_SERIAL63_MAX symbolic constant defines the maximum value of an
   *      object ID serial.
   *
   *      TODO: explain why it is 10 * 62 * (62* 62*62) * (62*62*62) * (62*62*62)
   */
#define RPS_SERIAL63_MAX ((uint64_t) 8392993658683402240)


  /*
   *      RPS_SERIAL63_DELTA - delta of object ID serial maxima and minima
   *
   *      The RPS_SERIAL63_DELTA symbolic constant defines the difference between
   *      the the maxiumum and minimum values of an object ID serial.
   *
   *      TODO: explain why it is RPS_SERIAL63_MAX - RPS_SERIAL63_MIN
   */
#define RPS_SERIAL63_DELTA (RPS_SERIAL63_MAX - RPS_SERIAL63_MIN)


  /****************************************************************************
   * Section: Object Bucket (rps_bucket)
   ****************************************************************************/


  /*
   *      RPS_BUCKET_MAX - maximum object buckets
   *
   *      The RPS_BUCKET_MAX symbolic constant defines the maximum object buckets.
   *      TODO: explain why it is 10 * 62, and its significance
   */
#define RPS_BUCKET_MAX ((size_t) 620)


  /****************************************************************************
   * Section: Object ID
   ****************************************************************************/


  /* TODO: object IDs are currently 128 bits, but may be reduced down to
   * 96 bits; accordingly, the rps_serial63 type will also need to be
   * redefined */
  typedef struct rps_objid_st
  {
    rps_serial63 hi;
    rps_serial63 lo;
  } rps_objid;


  /****************************************************************************
   * Section: Object
   ****************************************************************************/


  /* TODO: the object type is still incomplete, and its member fields are
   * not fully defined yet */
  typedef struct rps_object_st
  {
#warning "TODO: rps_object_st is still incomplete"
    /* some fields are needed before objid */
    const rps_objid objid;
    /* many fields are needed after objid */
  } rps_object;


  /****************************************************************************
   * Section: Typed Types (WIP)
   ****************************************************************************/

  /* corresponds to hash_tyBM */
  /* TODO: need to define */
  /* TODO: replace with class rps::Hash? */
  typedef struct rps_hash_st { } rps_hash;

  /* corresponds to gctyenum_BM; enumerates garbage collected types of
   * refpersys */
  /* TODO: replace with enum rps::ValType? */
  typedef enum RPS_GCTYPE_ENUM
  {
#warning TODO: RPS_GCTYPE_ENUM is still incomplete
    RPS_GCTYPE_INT = -1,   /* tagged integer */
    RPS_GCTYPE_NONE = 0,   /* nil */
    RPS_GCTYPE_STRING,     /* boxed string */
    RPS_GCTYPE_DOUBLE,     /* boxed double */
    RPS_GCTYPE_SET,        /* boxed set */
    RPS_GCTYPE_TUPLE,      /* boxed tuple */
    RPS_GCTYPE_NODE,       /* boxed node */
    RPS_GCTYPE_CLOSURE,    /* boxed closure */
    RPS_GCTYPE_OBJECT,     /* boxed object */
    RPS_GCTYPE_UNSPECIFIED /* unspecified value */

  } RPS_GCTYPE;


  /* corresponds to typedhead_tyBM */
  /* TODO: replace with class rps::TypedHead? */
  typedef struct rps_typedhead_st
  {
    RPS_GCTYPE gctype:24;
    uint32_t gchead:8;
    union
    {
      rps_hash hash;
      uint32_t rlen;
    };
  } rps_typedhead;

  /* corresponds to typedsize_tyBM */
  /* TODO: replace with class rps::TypedSize? */
  typedef struct rps_typedsz_st
  {
    rps_typedhead head;
    uint32_t size;
  } rps_typedsz;


  /* corresponds to typedforward_tyBM */
  /* TODO: replace with class rps::TypedForward? */
  typedef struct rps_typedfwd_st
  {
    rps_typedsz size;
    void *forward;
  } rps_typedfwd;


  /****************************************************************************
   * Section: Object Value
   ****************************************************************************/


  /* corresponds to Bismon's seqobval_stBM */
  typedef struct rps_value_st
  {
    rps_typedfwd forward;
    const rps_object *objects[RPS_FLEXIBLE_DIM];
  } rps_value;


  /****************************************************************************
   * Section: Object Value Tuple
   ****************************************************************************/


  /* corresponds to tupleval_tyBM */
  typedef rps_value rps_valuetuple;


  /****************************************************************************
   * Section: Object Value Set
   ****************************************************************************/


  /* corresponds to setval_tyBM */
  typedef rps_value rps_valueset;


  /* close support for C++ */
#if (defined __cplusplus)
}
#endif


namespace rps
{

// represents a 63-bit serial code
class Serial63
{
public:
  static const uint64_t MIN = RPS_SERIAL63_MIN;
  static const uint64_t MAX = RPS_SERIAL63_MAX;
  static const uint64_t DELTA = Serial63::MAX - Serial63::MIN;
  static const uint64_t MAXBUCKET = RPS_BUCKET_MAX;

  inline Serial63()
  {
    do
      {
        m_word = rps_random_uint64();
      }
    while (!this->valid());
  }

  inline ~Serial63()
  { }

  inline bool valid()
  {
    return m_word > Serial63::MIN && m_word < Serial63::MAX;
  }

  inline uint64_t bucket()
  {
    return m_word / (Serial63::DELTA / Serial63::MAXBUCKET);
  }

  int base62(char str);
  static Serial63 parse(const char *str);

private:
  uint64_t m_word;
};


// represents an object ID
class ObjectId
{
public:
  inline ObjectId()
    : m_hi()
    , m_lo()
  { }

  ~ObjectId();

private:
  Serial63 m_hi;
  Serial63 m_lo;
};


// represents a refpersys value
class Value
{
public:
// enumerates the refpersys value types
  enum Type
  {
    STRING, // scalar string
    DOUBLE, // scalar double
    SET,    // immutable set
    TUPLE   // immuatable tuple
  };

  inline Value(Value::Type type, intptr_t tagptr)
    : m_type(type)
    , m_tagptr(tagptr)
  { }

  inline ~Value()
  { }

private:
  Value::Type m_type;
  intptr_t m_tagptr;
};


// represents a value that is mark-swept by MPS
class MarkSweepValue : public Value
{
public:
  inline MarkSweepValue();
  inline ~MarkSweepValue();
};


// represents a value object
class ValueObject : public MarkSweepValue
{
public:
  inline ValueObject()
    : m_objid()
  { }

  inline ValueObject(ObjectId id)
    : m_objid(id)
  { }

  inline ~ValueObject()
  { }

private:
  ObjectId m_objid;
};


// represents a scalar value
class ScalarValue : public Value
{
public:
  inline ScalarValue();
  inline ~ScalarValue();
};


// represents an immutable value
class ImmutableValue : public Value
{
public:
  inline ImmutableValue();
  inline ~ImmutableValue();
};


// represents a sequence value (or sequence of values)
class SequenceValue : public ImmutableValue
{
public:
  inline SequenceValue();
  inline ~SequenceValue();

private:
  size_t m_size;
  ValueObject *m_objects[RPS_FLEXIBLE_DIM];
};


// represents a scalar string value
class StringValue : public ScalarValue
{
  uint32_t _strsize; // allocated size, in bytes, with a terminating 0 byte
  const char _strbytes[RPS_FLEXIBLE_DIM]; // actual size is _strsize
  inline StringValue();
public:
  static const StringValue* make(const char*);
};


// represents a scalar double value
class DoubleValue : public ScalarValue
{
public:
  inline DoubleValue();
  inline ~DoubleValue();
};


// represents a set, which is an immutable sequence value
class SetValue : public SequenceValue
{
public:
  inline SetValue();
  inline ~SetValue();
};


// represents a tuple, which is an immutable sequence value
class TupleValue : public SequenceValue
{
public:
  inline TupleValue();
  inline ~TupleValue();
};

} // namespace rps


#endif /* (!defined __REFPERSYS_TYPES_DEFINED) */

