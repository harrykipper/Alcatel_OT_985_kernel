
#ifndef __KERNTABLE_H
#define __KERNTABLE_H

#ifndef __LETYPES_H
#include "LETypes.h"
#endif

#include "LETypes.h"
//#include "LEFontInstance.h"
//#include "LEGlyphStorage.h"

#include <stdio.h>

U_NAMESPACE_BEGIN
struct PairInfo;
class  LEFontInstance;
class  LEGlyphStorage;

class U_LAYOUT_API KernTable
{
 private:
  le_uint16 coverage;
  le_uint16 nPairs;
  const PairInfo* pairs;
  const LEFontInstance* font;
  le_uint16 searchRange;
  le_uint16 entrySelector;
  le_uint16 rangeShift;

 public:
  KernTable(const LEFontInstance* font, const void* tableData);

  /*
   * Process the glyph positions.
   */
  void process(LEGlyphStorage& storage);
};

U_NAMESPACE_END

#endif
