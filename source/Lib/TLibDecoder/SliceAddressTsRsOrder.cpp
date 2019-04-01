


#include "SliceAddressTsRsOrder.h"

SliceAddressTsRsOrder::SliceAddressTsRsOrder()
:m_frameWidthInCtus(0)
, m_frameHeightInCtus(0)
, m_numCtusInFrame(0)
, m_numTileColumnsMinus1(0)
, m_numTileRowsMinus1(0)
, m_ctuTsToRsAddrMap(NULL)
, m_puiTileIdxMap(NULL)
, m_ctuRsToTsAddrMap(NULL)
{}


SliceAddressTsRsOrder::~SliceAddressTsRsOrder()
{
	destroy();
}



Void SliceAddressTsRsOrder::create(const TComSPS *sps, const TComPPS *pps)
{
	destroy();

	const Int iPicWidth = sps->getPicWidthInLumaSamples();
	const Int iPicHeight = sps->getPicHeightInLumaSamples();
	const UInt uiMaxCuWidth = sps->getMaxCUWidth();
	const UInt uiMaxCuHeight = sps->getMaxCUHeight();

	m_frameWidthInCtus = (iPicWidth %uiMaxCuWidth) ? iPicWidth / uiMaxCuWidth + 1 : iPicWidth / uiMaxCuWidth;
	m_frameHeightInCtus = (iPicHeight%uiMaxCuHeight) ? iPicHeight / uiMaxCuHeight + 1 : iPicHeight / uiMaxCuHeight;

	m_numCtusInFrame = m_frameWidthInCtus * m_frameHeightInCtus;

	m_ctuTsToRsAddrMap = new UInt[m_numCtusInFrame + 1];
	m_puiTileIdxMap = new UInt[m_numCtusInFrame];
	m_ctuRsToTsAddrMap = new UInt[m_numCtusInFrame + 1];

	for (UInt i = 0; i<m_numCtusInFrame; i++)
	{
		m_ctuTsToRsAddrMap[i] = i;
		m_ctuRsToTsAddrMap[i] = i;
	}

	xInitTiles(sps, pps);
	xInitCtuTsRsAddrMaps();
}

Void SliceAddressTsRsOrder::destroy()
{


	delete[] m_ctuTsToRsAddrMap;
	m_ctuTsToRsAddrMap = NULL;

	delete[] m_puiTileIdxMap;
	m_puiTileIdxMap = NULL;

	delete[] m_ctuRsToTsAddrMap;
	m_ctuRsToTsAddrMap = NULL;

}



Void SliceAddressTsRsOrder::xInitCtuTsRsAddrMaps()
{
	//generate the Coding Order Map and Inverse Coding Order Map
	for (Int ctuTsAddr = 0, ctuRsAddr = 0; ctuTsAddr<getNumberOfCtusInFrame(); ctuTsAddr++, ctuRsAddr = xCalculateNextCtuRSAddr(ctuRsAddr))
	{
		setCtuTsToRsAddrMap(ctuTsAddr, ctuRsAddr);
		setCtuRsToTsAddrMap(ctuRsAddr, ctuTsAddr);
	}
	setCtuTsToRsAddrMap(getNumberOfCtusInFrame(), getNumberOfCtusInFrame());
	setCtuRsToTsAddrMap(getNumberOfCtusInFrame(), getNumberOfCtusInFrame());
}

Void SliceAddressTsRsOrder::xInitTiles(const TComSPS *m_sps, const TComPPS *m_pps)
{
	//set NumColumnsMinus1 and NumRowsMinus1
	setNumTileColumnsMinus1(m_pps->getNumTileColumnsMinus1());
	setNumTileRowsMinus1(m_pps->getNumTileRowsMinus1());

	const Int numCols = m_pps->getNumTileColumnsMinus1() + 1;
	const Int numRows = m_pps->getNumTileRowsMinus1() + 1;
	const Int numTiles = numRows * numCols;

	// allocate memory for tile parameters
	m_tileParameters.resize(numTiles);

	if (m_pps->getTileUniformSpacingFlag())
	{
		//set width and height for each (uniform) tile
		for (Int row = 0; row < numRows; row++)
		{
			for (Int col = 0; col < numCols; col++)
			{
				const Int tileIdx = row * numCols + col;
				m_tileParameters[tileIdx].setTileWidthInCtus((col + 1)*getFrameWidthInCtus() / numCols - (col*getFrameWidthInCtus()) / numCols);
				m_tileParameters[tileIdx].setTileHeightInCtus((row + 1)*getFrameHeightInCtus() / numRows - (row*getFrameHeightInCtus()) / numRows);
			}
		}
	}
	else
	{
		//set the width for each tile
		for (Int row = 0; row < numRows; row++)
		{
			Int cumulativeTileWidth = 0;
			for (Int col = 0; col < getNumTileColumnsMinus1(); col++)
			{
				m_tileParameters[row * numCols + col].setTileWidthInCtus(m_pps->getTileColumnWidth(col));
				cumulativeTileWidth += m_pps->getTileColumnWidth(col);
			}
			m_tileParameters[row * numCols + getNumTileColumnsMinus1()].setTileWidthInCtus(getFrameWidthInCtus() - cumulativeTileWidth);
		}

		//set the height for each tile
		for (Int col = 0; col < numCols; col++)
		{
			Int cumulativeTileHeight = 0;
			for (Int row = 0; row < getNumTileRowsMinus1(); row++)
			{
				m_tileParameters[row * numCols + col].setTileHeightInCtus(m_pps->getTileRowHeight(row));
				cumulativeTileHeight += m_pps->getTileRowHeight(row);
			}
			m_tileParameters[getNumTileRowsMinus1() * numCols + col].setTileHeightInCtus(getFrameHeightInCtus() - cumulativeTileHeight);
		}
	}

	// Tile size check
	Int minWidth = 1;
	Int minHeight = 1;
	const Int profileIdc = m_sps->getPTL()->getGeneralPTL()->getProfileIdc();
	if (profileIdc == Profile::MAIN || profileIdc == Profile::MAIN10) //TODO: add more profiles to the tile-size check...
	{
		if (m_pps->getTilesEnabledFlag())
		{
			minHeight = 64 / m_sps->getMaxCUHeight();
			minWidth = 256 / m_sps->getMaxCUWidth();
		}
	}
	for (Int row = 0; row < numRows; row++)
	{
		for (Int col = 0; col < numCols; col++)
		{
			const Int tileIdx = row * numCols + col;
			assert(m_tileParameters[tileIdx].getTileWidthInCtus() >= minWidth);
			assert(m_tileParameters[tileIdx].getTileHeightInCtus() >= minHeight);
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
				rightEdgePosInCTU += m_tileParameters[row * numCols + i].getTileWidthInCtus();
			}
			m_tileParameters[tileIdx].setRightEdgePosInCtus(rightEdgePosInCTU - 1);

			//initialize the BottomEdgePosInCU for each tile
			Int bottomEdgePosInCTU = 0;
			for (Int i = 0; i <= row; i++)
			{
				bottomEdgePosInCTU += m_tileParameters[i * numCols + col].getTileHeightInCtus();
			}
			m_tileParameters[tileIdx].setBottomEdgePosInCtus(bottomEdgePosInCTU - 1);

			//initialize the FirstCUAddr for each tile
			m_tileParameters[tileIdx].setFirstCtuRsAddr((m_tileParameters[tileIdx].getBottomEdgePosInCtus() - m_tileParameters[tileIdx].getTileHeightInCtus() + 1) * getFrameWidthInCtus() +
				m_tileParameters[tileIdx].getRightEdgePosInCtus() - m_tileParameters[tileIdx].getTileWidthInCtus() + 1);
		}
	}

	Int  columnIdx = 0;
	Int  rowIdx = 0;

	//initialize the TileIdxMap
	for (Int i = 0; i<m_numCtusInFrame; i++)
	{
		for (Int col = 0; col < numCols; col++)
		{
			if (i % getFrameWidthInCtus() <= m_tileParameters[col].getRightEdgePosInCtus())
			{
				columnIdx = col;
				break;
			}
		}
		for (Int row = 0; row < numRows; row++)
		{
			if (i / getFrameWidthInCtus() <= m_tileParameters[row*numCols].getBottomEdgePosInCtus())
			{
				rowIdx = row;
				break;
			}
		}
		m_puiTileIdxMap[i] = rowIdx * numCols + columnIdx;
	}
}
UInt SliceAddressTsRsOrder::xCalculateNextCtuRSAddr(UInt currCtuRsAddr)
{
	UInt  nextCtuRsAddr;

	//get the tile index for the current CTU
	const UInt uiTileIdx = getTileIdxMap(currCtuRsAddr);

	//get the raster scan address for the next CTU
	if (currCtuRsAddr % m_frameWidthInCtus == getTComTile(uiTileIdx)->getRightEdgePosInCtus() && currCtuRsAddr / m_frameWidthInCtus == getTComTile(uiTileIdx)->getBottomEdgePosInCtus())
		//the current CTU is the last CTU of the tile
	{
		if (uiTileIdx + 1 == getNumTiles())
		{
			nextCtuRsAddr = m_numCtusInFrame;
		}
		else
		{
			nextCtuRsAddr = getTComTile(uiTileIdx + 1)->getFirstCtuRsAddr();
		}
	}
	else //the current CTU is not the last CTU of the tile
	{
		if (currCtuRsAddr % m_frameWidthInCtus == getTComTile(uiTileIdx)->getRightEdgePosInCtus())  //the current CTU is on the rightmost edge of the tile
		{
			nextCtuRsAddr = currCtuRsAddr + m_frameWidthInCtus - getTComTile(uiTileIdx)->getTileWidthInCtus() + 1;
		}
		else
		{
			nextCtuRsAddr = currCtuRsAddr + 1;
		}
	}

	return nextCtuRsAddr;
}


ExtTile::ExtTile()
: m_tileWidthInCtus(0)
, m_tileHeightInCtus(0)
, m_rightEdgePosInCtus(0)
, m_bottomEdgePosInCtus(0)
, m_firstCtuRsAddr(0)
{
}

ExtTile::~ExtTile()
{
}