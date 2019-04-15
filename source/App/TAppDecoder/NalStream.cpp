#include "NalStream.h"

NalStream::NalStream()
: mEntropyDecoder(TDecEntropy()),
  mCavlcDecoder(TDecCavlc()),
  mParameterSetManager(ParameterSetManager()),
  mStats(AnnexBStats())
{
}

NalStream::NalStream(const char* filename) 
  : mEntropyDecoder(TDecEntropy()),
  mCavlcDecoder(TDecCavlc()),
  mParameterSetManager(ParameterSetManager()),
  mStream(filename, std::ifstream::in | std::ifstream::binary)
{
  if (!mStream.is_open())
  {
    std::cerr << "Warning: File is not opened\n";
    exit(EXIT_FAILURE);
  }
}

Void NalStream::addFile(const char* filename)
{
  std::cout << filename << std::endl;
  mStream.open(filename, std::ifstream::in | std::ifstream::binary);

  if (!mStream.is_open())
  {
    std::cerr << "Warning: File is not opened\n";
    exit(EXIT_FAILURE);
  }

  mByteStream = new InputByteStream(mStream);
}





Void NalStream::readNALUnit(InputNALUnit& nalu)
{
  byteStreamNALUnit(*mByteStream, nalu.getBitstream().getFifo(), mStats);

  read(nalu);
  if (nalu.getBitstream().getFifo().empty())
  {
    std::cerr << "Waring: Attempt to decode an empty NAL unit\n";
  }

  std::cerr << "Entropy Decoding...\n";

  mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);
  mEntropyDecoder.setBitstream(&(nalu.getBitstream()));
    
  std::cout << nalUnitTypeToString(nalu.m_nalUnitType) << std::endl;
  switch (nalu.m_nalUnitType)
  {
  case NAL_UNIT_VPS:
  {
    TComVPS*    vps = new TComVPS();
    mEntropyDecoder.decodeVPS(vps);

    mParameterSetManager.storeVPS(vps, nalu.getBitstream().getFifo());
  }
  break;
  case NAL_UNIT_SPS:
  {
    TComSPS*		sps = new TComSPS();
    mEntropyDecoder.decodeSPS(sps);
    //Test
    sps->setPicWidthInLumaSamples(512);
    sps->setPicHeightInLumaSamples(320);

    mParameterSetManager.storeSPS(sps, nalu.getBitstream().getFifo());

    mExtSPSId = sps->getSPSId();
  }
  break;
  case NAL_UNIT_PPS:
  {
    TComPPS*		pps = new TComPPS();
    mEntropyDecoder.decodePPS(pps); 

    mParameterSetManager.storePPS(pps, nalu.getBitstream().getFifo());

    mExtPPSId = pps->getPPSId();
  }  
  break;
  case NAL_UNIT_PREFIX_SEI:
  {
  }
  break;
  case NAL_UNIT_CODED_SLICE_TRAIL_R:
  case NAL_UNIT_CODED_SLICE_TRAIL_N:
  case NAL_UNIT_CODED_SLICE_TSA_R:
  case NAL_UNIT_CODED_SLICE_TSA_N:
  case NAL_UNIT_CODED_SLICE_STSA_R:
  case NAL_UNIT_CODED_SLICE_STSA_N:
  case NAL_UNIT_CODED_SLICE_BLA_W_LP:
  case NAL_UNIT_CODED_SLICE_BLA_W_RADL:
  case NAL_UNIT_CODED_SLICE_BLA_N_LP:
  case NAL_UNIT_CODED_SLICE_IDR_W_RADL:
  case NAL_UNIT_CODED_SLICE_IDR_N_LP:
  case NAL_UNIT_CODED_SLICE_CRA:
  case NAL_UNIT_CODED_SLICE_RADL_N:
  case NAL_UNIT_CODED_SLICE_RADL_R:
  case NAL_UNIT_CODED_SLICE_RASL_N:
  case NAL_UNIT_CODED_SLICE_RASL_R:
  {
  }
  break;
  default:
    break;
  }
}

TComVPS* NalStream::getVPS(InputNALUnit& nalu)
{ 
  while (!mParameterSetManager.getFirstVPS())
  {
    readNALUnit(nalu);
  }
  return mParameterSetManager.getFirstVPS();
}

TComSPS* NalStream::getSPS(InputNALUnit& nalu)
{
  while (!mParameterSetManager.getFirstSPS())
  {
    readNALUnit(nalu);
  }
  return mParameterSetManager.getFirstSPS();
}

TComPPS* NalStream::getPPS(InputNALUnit& nalu)
{
  while (!mParameterSetManager.getFirstPPS())
  {
    readNALUnit(nalu);
  }
  return mParameterSetManager.getFirstPPS();
}

Void NalStream::getSliceNAL(InputNALUnit& nalu)
{
  readNALUnit(nalu);
  /*if (!mByteStream)
  {
    while (nalu.m_nalUnitType == NAL_UNIT_VPS ||
      nalu.m_nalUnitType == NAL_UNIT_PPS ||
      nalu.m_nalUnitType == NAL_UNIT_SPS)
    {
    }
  }*/
}