// Copyright (C) 2009, International Business Machines
// Corporation and others. All Rights Reserved.
//
// Copyright 2001 and onwards Google Inc.
// Author: Sanjay Ghemawat

// This code is a contribution of Google code, and the style used here is
// a compromise between the original Google code and the ICU coding guidelines.
// For example, data types are ICU-ified (size_t,int->int32_t),
// and API comments doxygen-ified, but function names and behavior are
// as in the original, if possible.
// Assertion-style error handling, not available in ICU, was changed to
// parameter "pinning" similar to UnicodeString.
//
// In addition, this is only a partial port of the original Google code,
// limited to what was needed so far. The (nearly) complete original code
// is in the ICU svn repository at icuhtml/trunk/design/strings/contrib
// (see ICU ticket 6765, r25517).

#ifndef __STRINGPIECE_H__
#define __STRINGPIECE_H__


#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/std_string.h"

// Arghh!  I wish C++ literals were "string".

U_NAMESPACE_BEGIN

class U_COMMON_API StringPiece : public UMemory {
 private:
  const char*   ptr_;
  int32_t       length_;

 public:
  /**
   * Default constructor, creates an empty StringPiece.
   * @draft ICU 4.2
   */
  StringPiece() : ptr_(NULL), length_(0) { }
  /**
   * Constructs from a NUL-terminated const char * pointer.
   * @param str a NUL-terminated const char * pointer
   * @draft ICU 4.2
   */
  StringPiece(const char* str);
#if U_HAVE_STD_STRING
  /**
   * Constructs from a std::string.
   * @draft ICU 4.2
   */
  StringPiece(const U_STD_NSQ string& str)
    : ptr_(str.data()), length_(static_cast<int32_t>(str.size())) { }
#endif
  /**
   * Constructs from a const char * pointer and a specified length.
   * @param offset a const char * pointer (need not be terminated)
   * @param len the length of the string; must be non-negative
   * @draft ICU 4.2
   */
  StringPiece(const char* offset, int32_t len) : ptr_(offset), length_(len) { }
  /**
   * Substring of another StringPiece.
   * @param x the other StringPiece
   * @param pos start position in x; must be non-negative and <= x.length().
   * @draft ICU 4.2
   */
  StringPiece(const StringPiece& x, int32_t pos);
  /**
   * Substring of another StringPiece.
   * @param x the other StringPiece
   * @param pos start position in x; must be non-negative and <= x.length().
   * @param len length of the substring;
   *            must be non-negative and will be pinned to at most x.length() - pos.
   * @draft ICU 4.2
   */
  StringPiece(const StringPiece& x, int32_t pos, int32_t len);

  /**
   * Returns the string pointer. May be NULL if it is empty.
   *
   * data() may return a pointer to a buffer with embedded NULs, and the
   * returned buffer may or may not be null terminated.  Therefore it is
   * typically a mistake to pass data() to a routine that expects a NUL
   * terminated string.
   * @return the string pointer
   * @draft ICU 4.2
   */
  const char* data() const { return ptr_; }
  /**
   * Returns the string length. Same as length().
   * @return the string length
   * @draft ICU 4.2
   */
  int32_t size() const { return length_; }
  /**
   * Returns the string length. Same as size().
   * @return the string length
   * @draft ICU 4.2
   */
  int32_t length() const { return length_; }
  /**
   * Returns whether the string is empty.
   * @return TRUE if the string is empty
   * @draft ICU 4.2
   */
  UBool empty() const { return length_ == 0; }

  /**
   * Sets to an empty string.
   * @draft ICU 4.2
   */
  void clear() { ptr_ = NULL; length_ = 0; }

  /**
   * Removes the first n string units.
   * @param n prefix length, must be non-negative and <=length()
   * @draft ICU 4.2
   */
  void remove_prefix(int32_t n) {
    if (n >= 0) {
      if (n > length_) {
        n = length_;
      }
      ptr_ += n;
      length_ -= n;
    }
  }

  /**
   * Removes the last n string units.
   * @param n suffix length, must be non-negative and <=length()
   * @draft ICU 4.2
   */
  void remove_suffix(int32_t n) {
    if (n >= 0) {
      if (n <= length_) {
        length_ -= n;
      } else {
        length_ = 0;
      }
    }
  }

  /**
   * Maximum integer, used as a default value for substring methods.
   * @draft ICU 4.2
   */
  static const int32_t npos = 0x7fffffff;

  /**
   * Returns a substring of this StringPiece.
   * @param pos start position; must be non-negative and <= length().
   * @param len length of the substring;
   *            must be non-negative and will be pinned to at most length() - pos.
   * @return the substring StringPiece
   * @draft ICU 4.2
   */
  StringPiece substr(int32_t pos, int32_t len = npos) const {
    return StringPiece(*this, pos, len);
  }
};

U_NAMESPACE_END

#endif  // __STRINGPIECE_H__
