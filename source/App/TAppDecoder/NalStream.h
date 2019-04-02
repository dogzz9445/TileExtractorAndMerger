#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

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

class NalStream
{
private:
  std::ifstream mStream;
  TDecEntropy mEntropyDecoder;
  TDecCavlc mCavlcDecoder;
  ParameterSetManager mParameterSetManager;
  AnnexBStats mStats;
  InputByteStream* mByteStream;
  //InputNALUnit mNalu;

public:
  NalStream();
  NalStream(const char* filename);
  ~NalStream()
  {
    if (mByteStream)
    {
      delete mByteStream;
    }
  }

  Void readNALUnit();
  Void addFile(const char* filename);

  TComVPS* getVPS();
  TComSPS* getSPS();
  TComPPS* getPPS();
  ParameterSetManager getParameterSetManager()                        { return mParameterSetManager; }
  void                setParameterSetManager(ParameterSetManager mP)  { mParameterSetManager = mP; }


};



#endif // _BITSTREAM_H_