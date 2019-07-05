#include "TilePartition.h"

Void TilePartitionManager::init()
{
  create();

  if (!mTileUniformSpacingFlag)
  {
    // FIXME:
    std::vector<Int> fixme1;
    std::vector<Int> fixme2;
    setTileWidths(fixme1);
    setTileHeights(fixme2);
  }
}

Void TilePartitionManager::create()
{
  destroy();

  const Int iPicWidth = mEntireWidth;
  const Int iPicHeight = mEntireHeight;
  const UInt uiMaxCuWidth = /*fixed*/64;
  const UInt uiMaxCuHeight = /*fixed*/64;

  mFrameWidthInCtus = (iPicWidth %uiMaxCuWidth) ? iPicWidth / uiMaxCuWidth + 1 : iPicWidth / uiMaxCuWidth;
  mFrameHeightInCtus = (iPicHeight%uiMaxCuHeight) ? iPicHeight / uiMaxCuHeight + 1 : iPicHeight / uiMaxCuHeight;

  mNumCtusInFrame = mFrameWidthInCtus * mFrameHeightInCtus;
}

Void TilePartitionManager::destroy()
{

}

Void TilePartitionManager::calculate()
{
  const Int numRows = mNumTilesInRow;
  const Int numCols = mNumTilesInColumn;
  const Int numTiles = numRows * numCols;

  mTileParameters.resize(numTiles);

  if (mTileUniformSpacingFlag)
  {
    //set width and height for each (uniform) tile
    for (Int row = 0; row < numRows; row++)
    {
      for (Int col = 0; col < numCols; col++)
      {
        const Int tileIdx = row * numCols + col;
        mTileParameters[tileIdx].setTileWidthInCtus((col + 1)*getFrameWidthInCtus() / numCols - (col*getFrameWidthInCtus()) / numCols);
        mTileParameters[tileIdx].setTileHeightInCtus((row + 1)*getFrameHeightInCtus() / numRows - (row*getFrameHeightInCtus()) / numRows);
      }
    }
  }
  else
  {
    //set the width for each tile
    for (Int row = 0; row < numRows; row++)
    {
      Int cumulativeTileWidth = 0;
      for (Int col = 0; col < numCols - 1; col++)
      {
        mTileParameters[row * numCols + col].setTileWidthInCtus(getTileColumnWidth(col));
        cumulativeTileWidth += getTileColumnWidth(col);
      }
      mTileParameters[row * numCols + getNumTileColumnsMinus1()].setTileWidthInCtus(getFrameWidthInCtus() - cumulativeTileWidth);
    }

    //set the height for each tile
    for (Int col = 0; col < numCols; col++)
    {
      Int cumulativeTileHeight = 0;
      for (Int row = 0; row < getNumTileRowsMinus1(); row++)
      {
        mTileParameters[row * numCols + col].setTileHeightInCtus(getTileRowHeight(row));
        cumulativeTileHeight += getTileRowHeight(row);
      }
      mTileParameters[getNumTileRowsMinus1() * numCols + col].setTileHeightInCtus(getFrameHeightInCtus() - cumulativeTileHeight);
    }
  }

  // Tile size check
  Int minWidth = 4;
  Int minHeight = 1;
  for (Int row = 0; row < numRows; row++)
  {
    for (Int col = 0; col < numCols; col++)
    {
      const Int tileIdx = row * numCols + col;
      assert(mTileParameters[tileIdx].getTileWidthInCtus() >= minWidth);
      assert(mTileParameters[tileIdx].getTileHeightInCtus() >= minHeight);
    }
  }

  //initialize each tile of the current picture
  for (Int row = 0; row < numRows; row++)
  {
    for (Int col = 0; col < numCols; col++)
    {
      const Int tileIdx = row * numCols + col;

      //initialize the RightEdgePosInCU for each tile
      Int rightEdgePosInCTU = 0;
      for (Int i = 0; i <= col; i++)
      {
        rightEdgePosInCTU += mTileParameters[row * numCols + i].getTileWidthInCtus();
      }
      mTileParameters[tileIdx].setRightEdgePosInCtus(rightEdgePosInCTU - 1);

      //initialize the BottomEdgePosInCU for each tile
      Int bottomEdgePosInCTU = 0;
      for (Int i = 0; i <= row; i++)
      {
        bottomEdgePosInCTU += mTileParameters[i * numCols + col].getTileHeightInCtus();
      }
      mTileParameters[tileIdx].setBottomEdgePosInCtus(bottomEdgePosInCTU - 1);

      //initialize the FirstCUAddr for each tile
      mTileParameters[tileIdx].setFirstCtuRsAddr((mTileParameters[tileIdx].getBottomEdgePosInCtus() - mTileParameters[tileIdx].getTileHeightInCtus() + 1) * getFrameWidthInCtus() +
        mTileParameters[tileIdx].getRightEdgePosInCtus() - mTileParameters[tileIdx].getTileWidthInCtus() + 1);
    }
  }

  Int  columnIdx = 0;
  Int  rowIdx = 0;

  //initialize the TileIdxMap
  for (Int i = 0; i< mNumCtusInFrame; i++)
  {
    for (Int col = 0; col < numCols; col++)
    {
      if (i % getFrameWidthInCtus() <= mTileParameters[col].getRightEdgePosInCtus())
      {
        columnIdx = col;
        break;
      }
    }
    for (Int row = 0; row < numRows; row++)
    {
      if (i / getFrameWidthInCtus() <= mTileParameters[row*numCols].getBottomEdgePosInCtus())
      {
        rowIdx = row;
        break;
      }
    }
  }
}
