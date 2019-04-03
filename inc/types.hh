#if (!defined RPS_TYPES_DEFINED)
#define RPS_TYPES_DEFINED

// 96-bit object ID represented in base 62
class RpsObjectId
{
public:
  static constexpr RADIX = 62;
  static constexpr DIGITS[] = "0123456789"
                              "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  // checks whether this object ID is equal to another
  // uses concept that XOR is identical to !=
  bool operator == (const RpsObjectId& cmp)
  {
    return !(_hi ^ cmp._hi) && !(_lo ^ cmp._lo);
  }

  // checks whether this object ID is not equal to another
  bool operator != (const RpsObjectId& cmp)
  {
    return !(*this == cmp);
  }

  // checks whether this object ID is greater than another
  bool operator > (const RpsObjectId& cmp)
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
  bool operator < (const RpsObjectId& cmp)
  {
    return cmp > *this;
  }

  // checks whether this object ID is greater than or equal to another
  bool operator >= (const RpsObjectId& cmp)
  {
    return !(*this < cmp);
  }

  // checks whether this object ID is less than or equal to another
  bool operator <= (const RpsObjectId& cmp)
  {
    return !(*this > cmp);
  }

private:
  uint64_t _hi; // most significant 64-bits
  uint32_t _lo; // least significant 32-bits
}; // end class RpsObjectId

#endif // !defined RPS_TYPES_DEFINED

