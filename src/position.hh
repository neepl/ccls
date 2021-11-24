// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "utils.hh"

#include <stdint.h>
#include <string>

namespace ccls {
struct Pos {
#ifdef LINENUM_32BIT
  using LineNumType = int32_t;
  static LineNumType const lineNumMax = INT32_MAX;
#else
  using LineNumType = uint16_t;
  static LineNumType const lineNumMax = UINT16_MAX;
#endif
  LineNumType line = 0;
  int16_t column = -1;

  static Pos fromString(const std::string &encoded);

  bool valid() const { return column >= 0; }
  std::string toString();

  // Compare two Positions and check if they are equal. Ignores the value of
  // |interesting|.
  bool operator==(const Pos &o) const {
    return line == o.line && column == o.column;
  }
  bool operator<(const Pos &o) const {
    if (line != o.line)
      return line < o.line;
    return column < o.column;
  }
  bool operator<=(const Pos &o) const { return !(o < *this); }
};

struct Range {
  Pos start;
  Pos end;

  static Range fromString(const std::string &encoded);

  bool valid() const { return start.valid(); }
  bool contains(int line, int column) const;

  std::string toString();

  bool operator==(const Range &o) const {
    return start == o.start && end == o.end;
  }
  bool operator<(const Range &o) const {
    return !(start == o.start) ? start < o.start : end < o.end;
  }
};

// reflection
struct JsonReader;
struct JsonWriter;
struct BinaryReader;
struct BinaryWriter;

void reflect(JsonReader &visitor, Pos &value);
void reflect(JsonReader &visitor, Range &value);
void reflect(JsonWriter &visitor, Pos &value);
void reflect(JsonWriter &visitor, Range &value);
void reflect(BinaryReader &visitor, Pos &value);
void reflect(BinaryReader &visitor, Range &value);
void reflect(BinaryWriter &visitor, Pos &value);
void reflect(BinaryWriter &visitor, Range &value);
} // namespace ccls

namespace std {
template <> struct hash<ccls::Range> {
  #ifdef LINENUM_32BIT
  std::size_t operator()(ccls::Range x) const {
    static_assert(sizeof(ccls::Range) == 16);

    /* Hash tries to combine things that are likely not to collide
       0000000000000 startcol15...0 startline31..16 startline15...0
        endline15..0 el23..1631..24     ec7..015..8 000000000000000
    */
    int16_t endColSwap = ((x.end.column << 8) & 0xff00) | ((x.end.column >> 8)&0x00ff);
    int16_t endLineHSwap = ((x.end.line >> 8) & 0xff00) | ((x.end.line >> 24)&0x00ff);
    uint64_t u64HashLower = (((uint64_t) x.start.column) << 32) | x.start.line;
    uint64_t u64HashUpper = (((uint64_t)(x.end.line & 0xffff)) << 48) | (((uint64_t)endLineHSwap) << 32) | (((uint64_t)endColSwap) << 16);
    return hash<uint64_t>()(u64HashLower ^ u64HashUpper);
  }
  #else
  std::size_t operator()(ccls::Range x) const {
    union {
      ccls::Range range;
      uint64_t u64;
    } u{x};
    static_assert(sizeof(ccls::Range) == 8);
    return hash<uint64_t>()(u.u64);
  }
  #endif // LINENUM_32BIT
};
} // namespace std

MAKE_HASHABLE(ccls::Pos, t.line, t.column);
