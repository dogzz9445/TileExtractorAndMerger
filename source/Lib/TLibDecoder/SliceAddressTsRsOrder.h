#ifndef _SLICEADDRESSTSRSORDER_H_
#define _SLICEADDRESSTSRSORDER_H_
#include "TLibCommon/TComSlice.h"

class ExtTile
{
private:
	UInt      m_tileWidthInCtus;
	UInt      m_tileHeightInCtus;
	UInt      m_rightEdgePosInCtus;
	UInt      m_bottomEdgePosInCtus;
	UInt      m_firstCtuRsAddr;

public:
	ExtTile();
	virtual ~ExtTile();

	Void      setTileWidthInCtus(UInt i)            { m_tileWidthInCtus = i; }
	UInt      getTileWidthInCtus() const              { return m_tileWidthInCtus; }
	Void      setTileHeightInCtus(UInt i)            { m_tileHeightInCtus = i; }
	UInt      getTileHeightInCtus() const              { return m_tileHeightInCtus; }
	Void      setRightEdgePosInCtus(UInt i)            { m_rightEdgePosInCtus = i; }
	UInt      getRightEdgePosInCtus() const              { return m_rightEdgePosInCtus; }
	Void      setBottomEdgePosInCtus(UInt i)            { m_bottomEdgePosInCtus = i; }
	UInt      getBottomEdgePosInCtus() const              { return m_bottomEdgePosInCtus; }
	Void      setFirstCtuRsAddr(UInt i)            { m_firstCtuRsAddr = i; }
	UInt      getFirstCtuRsAddr() const              { return m_firstCtuRsAddr; }
};

/// picture symbol class
class SliceAddressTsRsOrder
{
private:
	UInt          m_frameWidthInCtus;
	UInt          m_frameHeightInCtus;
	UInt          m_numCtusInFrame;


	Int           m_numTileColumnsMinus1;
	Int           m_numTileRowsMinus1;
	std::vector<ExtTile> m_tileParameters;
	UInt*         m_ctuTsToRsAddrMap;    ///< for a given TS (Tile-Scan; coding order) address, returns the RS (Raster-Scan) address. cf CtbAddrTsToRs in specification.
	UInt*         m_puiTileIdxMap;       ///< the map of the tile index relative to CTU raster scan address
	UInt*         m_ctuRsToTsAddrMap;    ///< for a given RS (Raster-Scan) address, returns the TS (Tile-Scan; coding order) address. cf CtbAddrRsToTs in specification.


	Void               xInitTiles(const TComSPS *m_sps, const TComPPS *pps);
	Void               xInitCtuTsRsAddrMaps();
	Void               setNumTileColumnsMinus1(Int i)                      { m_numTileColumnsMinus1 = i; }
	Void               setNumTileRowsMinus1(Int i)                         { m_numTileRowsMinus1 = i; }
	Void               setCtuTsToRsAddrMap(Int ctuTsAddr, Int ctuRsAddr)   { *(m_ctuTsToRsAddrMap + ctuTsAddr) = ctuRsAddr; }
	Void               setCtuRsToTsAddrMap(Int ctuRsAddr, Int ctuTsOrder)  { *(m_ctuRsToTsAddrMap + ctuRsAddr) = ctuTsOrder; }

public:

	Void               create(const TComSPS *sps, const TComPPS *pps);

	Void               destroy();

	SliceAddressTsRsOrder();
	~SliceAddressTsRsOrder();


	UInt               getFrameWidthInCtus() const                           { return m_frameWidthInCtus; }
	UInt               getFrameHeightInCtus() const                          { return m_frameHeightInCtus; }
	UInt               getNumberOfCtusInFrame() const                        { return m_numCtusInFrame; }


	Int                getNumTileColumnsMinus1() const                       { return m_numTileColumnsMinus1; }
	Int                getNumTileRowsMinus1() const                          { return m_numTileRowsMinus1; }
	Int                getNumTiles() const                                   { return (m_numTileRowsMinus1 + 1)*(m_numTileColumnsMinus1 + 1); }
	ExtTile*          getTComTile(UInt tileIdx)                         { return &(m_tileParameters[tileIdx]); }
	const ExtTile*    getTComTile(UInt tileIdx) const                   { return &(m_tileParameters[tileIdx]); }
	UInt               getCtuTsToRsAddrMap(Int ctuTsAddr) const            { return *(m_ctuTsToRsAddrMap + (ctuTsAddr >= m_numCtusInFrame ? m_numCtusInFrame : ctuTsAddr)); }
	UInt               getTileIdxMap(Int ctuRsAddr) const                  { return *(m_puiTileIdxMap + ctuRsAddr); }
	UInt               getCtuRsToTsAddrMap(Int ctuRsAddr) const            { return *(m_ctuRsToTsAddrMap + (ctuRsAddr >= m_numCtusInFrame ? m_numCtusInFrame : ctuRsAddr)); }

protected:
	UInt               xCalculateNextCtuRSAddr(UInt uiCurrCtuRSAddr);

};// END CLASS DEFINITION TComPicSym

#endif // _SLICEADDRESSTSRSORDER_H_