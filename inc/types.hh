#if (!defined RPS_TYPES_DEFINED)
#define RPS_TYPES_DEFINED


#include <cassert>
#include "util.h"


// 96-bit object ID represented in base 62
class RpsObjectId
{
public:
  static constexpr RADIX = 62;
  static constexpr DIGITS[] = "0123456789"
                              "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static constexpr MSB_MIN = (uint64_t) 62 * 62 * 62;
  static constexpr MSB_MAX = (uint64_t) 10 * 62 * MSB_MIN * MSB_MIN * MSB_MIN;
  static constexpr LSB_MIN = (uint32_t) 62 * 62;
  static constexpr LSB_MAX = (uint32_t) 62 * LSB_MIN * LSB_MIN * LSB_MIN;


  // generates a random object ID
  static RpsObjectId generate_random()
  {
    while (true)
      {
        auto msb = RpsRandom::generate64_nonzero() % RpsObjectId::MSB_MAX;
        if (rps_unlikely (msb < MSB_MIN))
          {
            continue;
          }

        auto lsb = RpsRandom::generate32_nonzero() % RpsObjectId::LSB_MAX;
        if (rps_unlikely (lsb < LSB_MIN))
          {
            continue;
          }

        auto oid = RpsObjectId(msb, lsb);
        if (rps_unlikely (oid.get_hash() == 0))
          {
            continue;
          }

        return oid;
      }
  }


  // default constructor
  RpsObjectId()
    : RpsObjectId(0, 0)
  { }

  // overload constructor
  RpsObjectId(uint64_t msb, uint32_t lsb)
    : _hi(msb)
    , _lo(lsb)
  {
    assert((msb == 0 && lsb == 0) || get_hash() != 0);
  }

  // overloaded constructor
  RpsObjectId(std::nullptr_t)
    : RpsObjectId(RpsObjectId::generate_random())
  { }

  // default copy constructor
  RpsObjectId(const RpsObjectId &rhs)
    : RpsObjectId(rhs.get_msb_64(), rhs.get_lsb_32())
  { }

  // checks whether this object ID is equal to another
  // uses concept that XOR is identical to !=
  bool operator == (const RpsObjectId &cmp)
  {
    return (_hi == cmp._hi) && (_lo == cmp._lo);
  }

  // checks whether this object ID is not equal to another
  bool operator != (const RpsObjectId &cmp)
  {
    return !(*this == cmp);
  }

  // checks whether this object ID is greater than another
  bool operator > (const RpsObjectId &cmp)
  {
    if (_hi < cmp._hi)
      {
        return false;
      }
    else if (_hi > cmp._hi)
      {
        return true;
      }
    else
      {
        return _lo > cmp._lo;
      }
  }

  // checks whether this object ID is less than another
  bool operator < (const RpsObjectId &cmp)
  {
    return cmp > *this;
  }

  // checks whether this object ID is greater than or equal to another
  bool operator >= (const RpsObjectId &cmp)
  {
    return !(*this < cmp);
  }

  // checks whether this object ID is less than or equal to another
  bool operator <= (const RpsObjectId &cmp)
  {
    return !(*this > cmp);
  }

  // gets the most significant 64-bits
  uint64_t get_msb_64() const
  {
    return _hi;
  }

  // gets the least significant 32-bits
  uint32_t get_lsb_32() const
  {
    return _lo;
  }

  // gets the hash value
  uint32_t get_hash() const
  {
    return (_hi % 2147473837) + ((_hi >> 32) ^ (_lo * 17 + 201151));
  }

private:
  uint64_t _hi; // most significant 64-bits
  uint32_t _lo; // least significant 32-bits
}; // end class RpsObjectId


// represents a memory zone in the MPS AMS pool
class RpsMarkSweepZone;


// represents the MPS memory zone allocated for RpsObject instances
class RpsObjectZone : public RpsMarkSweepZone
{
public:
  uint32_t get_hash() const
  {
    return _id.get_hash();
  }

  // compares an object zone instance with another for equality
  // this function is required since we can't overload the
  // equality operator with RpsObjectZone*
  bool is_eq(const RpsObjectZone* rhs)
  {
    if (!rhs)
      {
        return false;
      }

    if (this != rhs && _id != rhs->_id)
      {
        return false;
      }

    return true;
  }

  // overloaded equality operator
  bool operator == (RpsObjectZone& rhs) const
  {
    return is_eq(&rhs);
  }

  // overloaded inequality operator
  bool operator != (RpsObjectZone& rhs) const
  {
    return !is_eq(&rhs);
  }
private:
  RpsObjectId _id;

  // private constructor
  RpsObjectZone(RpsObjectID id)
    : _id(id)
  { }
};


// represents a reference to a Refpersys object
class RpsObjectReference
{
public:

  // default constructor
  RpsObject(RpsObjectZone *zone = nullptr)
    : _zone(zone)
  { }

  // overloaded functor
  operator bool () const
  {
    return _zone != nullptr;
  }

  // overloaded negation operator
  operator bool ! () const
  {
    return _zone == nullptr
  }

  // overloaded cast operator
  operator const RpsObjectZone* () const
  {
    return get_zone();
  }

  // overloaded cast operator
  operator RpsObjectZone* () const
  {
    return get_zone();
  }

  // overloaded cast operator
  operator RpsObjectZone* ()
  {
    return get_zone();
  }

  // overloaded indirection operator
  RpsObjectZone* operator -> () const
  {
    return get_zone();
  }

  // overloaded indirection operator
  RpsObjectZone* operator -> ()
  {
    return get_zone();
  }

  // accessor to get pointer to MPS memory zone for objects
  RpsObjectZone *get_zone() const
  {
    assert(_zone != nullptr);
    return _zone;
  }

  // mutator to set pointer to MPS memory zone for objects
  void set_zone(RpsObjectZone *zone)
  {
    assert(zone != nullptr);
    _zone = zone;
  }

private:
  RpsObjectZone *zone; // pointer to MPS memory zone for objects
};


// represents the data of a Refpersys value
class RpsValueData;


// represents a reference to a Refpersys value
class RpsValueRef
{
  friend class RpsValueZone;

private:
  union
  {
    RpsValueData* _valdata;
    intptr_t _intdata;
  };

public:
  RpsValueRef()
    : _valdata(nullptr)
  { }

  RpsValueRef(const RpsValueData *valdata)
    : _valdata(valdata)
  { }

  RpsValueRef(int64_t intdata)
    : _intdata(intdata)
  { }

  RpsValueRef(const RpsValueRef& src)
    : _valdata(src._valdata)
  { }

  RpsValueRef(const RpsValueRef& src)
    : _intdata(src._intdata)
  { }

  RpsValueRef& operator= (const RpsValueRef& rhs)
  {
    _valdata = rhs._valdata;
    return *this;
  };

  // TODO: need to add move assignment operator; Abhishek is still trying to
  // understand the concept of move assignment
  RpsValueRef& operator= (RpsValue&& rhs) noexcept

  bool is_int() const
  {
    // QUERY: Abhishek thinks that this checks for the least significant bit;
    // does that mean that we are considering pointers with their least bit set
    // as integers? I was under (perhaps incorrectly) the impression that we
    // would be using the MSB to indicate whether a pointer is an integer
    return (_intdata & 0x1) != 0;
  };

  bool is_value() const
  {
    return !is_int();
  }

  bool has_value() const
  {
    return is_value() && _valdata != nullptr;
  }
} // end of RpsValueRef


#endif // !defined RPS_TYPES_DEFINED

