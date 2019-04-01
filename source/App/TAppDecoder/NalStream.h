#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include <iostream>
#include <vector>
#include <map>
#include <assert.h>

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
  std::istream& mNalStream;
  TDecEntropy mEntropyDecoder;
  TDecCavlc mCavlcDecoder;
  InputByteStream mByteStream;
  ParameterSetManager mParameterSetManager;
  AnnexBStats mStats;
  InputNALUnit mNalu;
  

public:
  NalStream();
  NalStream(std::istream& istream);
  virtual ~NalStream();

  InputNALUnit readNALUnit();

};



#endif // _BITSTREAM_H_