#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <fstream>

#include "TLibDecoder/TEncEntropy.h"
#include "TLibDecoder/TEncCavlc.h"
#include "TLibDecoder/TEncSbac.h"
#include "TLibDecoder/SyntaxElementParser.h"
#include "TLibDecoder/SEIread.h"
#include "TLibDecoder/TDecEntropy.h"
#include "TLibDecoder/TDecSbac.h"
#include "TLibDecoder/TDecCAVLC.h"
#include "TLibDecoder/SliceAddressTsRsOrder.h"

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"
#include "TLibEncoder/AnnexBwrite.h"
#include "TLibEncoder/NALwrite.h"

static const UChar emulation_prevention_three_byte[] = { 3 };

class NalStream
{
private:
  Int                 mTileId;

  std::ifstream       mStream;
  TDecEntropy         mEntropyDecoder;
  TEncEntropy         mEntropyCoder;
  TDecCavlc           mCavlcDecoder;
  TEncCavlc           mCavlcEncoder;
  AnnexBStats         mStats;
  InputByteStream*    mByteStream;
  InputNALUnit        nalu;
  std::fstream*       outStream;
  TComVPS*            inVPS;
  TComSPS*            inSPS;
  TComPPS*            inPPS;
  TComVPS*            emVPS;
  TComSPS*            emSPS;
  TComPPS*            emPPS;
  AccessUnit          accessUnit;
  ParameterSetManager* mParameterSetManager;
  ParameterSetManager* mOriParameterSetManager;
  //InputNALUnit mNalu;

  std::vector<Int>    mTileWidths;
  std::vector<Int>    mTileHeights;
  std::vector<Int>    mTileAddresses;

  Int mExtVPSId;
  Int mExtSPSId;
  Int mExtPPSId;

public:
  NalStream();
  NalStream(const char* filename);
  ~NalStream()
  {
    if (mByteStream)
    {
      delete mByteStream;
    }
    if (mOriParameterSetManager)
    {
      delete mOriParameterSetManager;
    }
    if (emVPS)
    {
      delete emVPS;
    }
    if (emPPS)
    {
      delete emPPS;
    }
    if(emSPS)
    {
      delete emSPS;
    }
  }

  Void readNALUnit();
  Void addFile(const char* filename);
  Void setOutputStream(std::fstream &out) { outStream = &out; }
  Void setParameterPointers(TComVPS*, TComSPS*, TComPPS*);
  Void setTileResolution(std::vector<Int>& tileAddresses, std::vector<Int>& tileWidths, std::vector<Int>& tileHeights);
  Void setParameterSetManager(ParameterSetManager* para) { mParameterSetManager = para; }

  Void                  setTileId(Int tileId) { mTileId = tileId; }
  Int                   getTileId() { return mTileId; }
  TComVPS*              getVPS();
  TComSPS*              getSPS();
  TComPPS*              getPPS();

};

std::size_t addEmulationPreventionByte(vector<uint8_t>& outputBuffer, vector<uint8_t>& rbsp);


#endif // _BITSTREAM_H_