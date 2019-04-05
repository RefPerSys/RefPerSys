#if (!defined RPS_TYPES_DEFINED)
#define RPS_TYPES_DEFINED


#include <cassert>


// 96-bit object ID represented in base 62
class RpsObjectId
{
public:
  static constexpr RADIX = 62;
  static constexpr DIGITS[] = "0123456789"
                              "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // default constructor
  RpsObjectId()
          : RpsObjectId(0, 0)
  { }

  // overload constructor
  RpsObjectId(uint64_t msb = 0, uint32_t lsb = 0)
          : _hi(msb)
          , _lo(lsb)
  {
          assert((msb == 0 && lsb == 0) || hash() != 0);
  }

  // overloaded constructor
  RpsObjectId(std::nullptr_t)
          : RpsObjectId()
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

#endif // !defined RPS_TYPES_DEFINED

