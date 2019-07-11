#ifndef _TILE_PARTITION_H_
#define _TILE_PARTITION_H_

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <fstream>

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"

#include "TLibDecoder/TEncEntropy.h"
#include "TLibDecoder/TEncCavlc.h"
#include "TLibDecoder/TEncSbac.h"
#include "TLibDecoder/SyntaxElementParser.h"
#include "TLibDecoder/SEIread.h"
#include "TLibDecoder/TDecEntropy.h"
#include "TLibDecoder/TDecSbac.h"
#include "TLibDecoder/TDecCAVLC.h"
#include "TLibDecoder/SliceAddressTsRsOrder.h"

#define ENTIRE_WIDTH 3840
#define ENTIRE_HEIGHT 1920

// PROFILE이 무조건 MAIN 또는 MAIN10 프로파일이어야한다.

class TilePartitionManager
{
private:
  std::vector<ExtTile> mTileParameters;

  Int   mEntireWidth;
  Int   mEntireHeight;
  Int   mNumTiles;
  Int   mNumTilesInColumn;
  Int   mNumTilesInRow;
  Bool  mTileEnabledFlag;
  Bool  mTileUniformSpacingFlag;

  Int mFrameWidthInCtus;
  Int mFrameHeightInCtus;
  
  Int mNumCtusInFrame;

  std::vector<Int> mTileWidths;
  std::vector<Int> mTileHeights;

public:
  TilePartitionManager() {};
  ~TilePartitionManager()
  {
    destroy();
  };

  // 무조건 인풋으로 들어와야하는 값들
  // mEntireWidth
  // mEntireHeight
  // mNumTilesInColumn
  // mNumTilesInRow
  // mTileUniformSpacingFlag
  // if (mTileUniformSpacingFlag)
  //     mTileWidths, mTileHeights
  Void init();

  Void create();
  Void destroy();

  Void calculate();

  Void setEntireWidth(Int value) { mEntireWidth = value; }
  Void setEntireHeight(Int value) { mEntireHeight = value; }
  Void setNumTiles(Int value) { mNumTiles = value; }
  Void setNumTilesInColumn(Int value) { mNumTilesInColumn = value; }
  Void setNumTilesInRow(Int value) { mNumTilesInRow = value; }
  Void setTileEnabledFlag(Bool flag) { mTileEnabledFlag = flag; }
  Void setTileUniformSpacingFlag(Bool flag) { mTileUniformSpacingFlag = flag; }

  Void setTileWidths(std::vector<Int> widths) { mTileWidths.resize(mNumTilesInColumn); mTileWidths = widths; }
  Void setTileHeights(std::vector<Int> heights) { mTileHeights.resize(mNumTilesInRow); mTileHeights = heights; }

  const Int getEntireWidth() { return mEntireWidth; }
  const Int getEntireHeight() { return mEntireHeight; }
  const Int getNumTiles() { return mNumTiles; }
  const Int getNumTilesInColumn() { return mNumTilesInColumn; }
  const Int getNumTilesInRow() { return mNumTilesInRow; }
  const Bool getTileEnabledFlag() { return mTileEnabledFlag; }
  const Bool getTileUniformSpacingFlag() { return mTileUniformSpacingFlag; }

  const Int getTileWidth(Int idx) { return mTileWidths[idx]; }
  const Int getTileHeight(Int idx) { return mTileHeights[idx]; }

  const Int getFrameWidthInCtus() { return mFrameWidthInCtus; }
  const Int getFrameHeightInCtus() { return mFrameHeightInCtus; }

  const Int getNumTileColumnsMinus1() { return mNumTilesInColumn - 1; }
  const Int getNumTileRowsMinus1() { return mNumTilesInRow - 1; }

  UInt getTileRowHeight(UInt rowidx) { return mTileHeights[rowidx]; }
  UInt getTileColumnWidth(UInt colidx) { return mTileWidths[colidx]; }
  

};

#endif // _TILE_PARTITION_H_