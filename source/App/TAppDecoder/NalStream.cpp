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
  mStream(filename, std::ifstream::in | std::ifstream::binary))
{
  if (!mStream.is_open())
  {
    std::cerr << "Warning: File is not opened\n";
    exit(1);
  }
}

Void NalStream::addFile(const char* filename)
{
  if (mStream.is_open())
  {
    mStream.close();
    mStream.clear();
  }

  mStream.open(filename, std::ifstream::in | std::ifstream::binary);

  if (!mStream.is_open)
  {
    std::cerr << "Warning: File is not opened\n";
    exit(1);
  }

  mByteStream = new InputByteStream(mStream);
}



Void NalStream::readNALUnit()
{
  InputNALUnit nalu;

  byteStreamNALUnit(*mByteStream, nalu.getBitstream().getFifo(), mStats);

  read(nalu);
  if (nalu.getBitstream().getFifo().empty())
  {
    std::cerr << "Waring: Attempt to decode an empty NAL unit\n";
  }
  mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);
  mEntropyDecoder.setBitstream(&(nalu.getBitstream()));

  switch (nalu.m_nalUnitType)
  {
  case NAL_UNIT_VPS:
  {
    TComVPS*    vps = new TComVPS();
    mEntropyDecoder.decodeVPS(vps);

    mParameterSetManager.storeVPS(vps, nalu.getBitstream().getFifo());
  }
  case NAL_UNIT_SPS:
  {
    TComSPS*		sps = new TComSPS();
    mEntropyDecoder.decodeSPS(sps);

    mParameterSetManager.storeSPS(sps, nalu.getBitstream().getFifo());
  }
  break;
  case NAL_UNIT_PPS:
  {
    TComPPS*		pps = new TComPPS();
    mEntropyDecoder.decodePPS(pps); 

    mParameterSetManager.storePPS(pps, nalu.getBitstream().getFifo());
  }
  break;
  case NAL_UNIT_PREFIX_SEI:
  {
    // FIXME:
    /*if (m_seiReader.parseSEImessage(*sei, &(nalu.getBitstream()), m_pSEIOutputStream, bitsSliceSegmentAddress))
    {
      replaceParameter(extractFile, *sei, m_mctsEisIdTarget, m_mctsSetIdxTarget, m_parameterSetManager);
      m_manageSliceAddress.create(m_parameterSetManager.getSPS(m_extSPSId), m_parameterSetManager.getPPS(m_extPPSId));
    }
    else
    {
      vector<uint8_t> outputBuffer;
      std::size_t outputAmount = 0;
      outputAmount = addEmulationPreventionByte(outputBuffer, nalu.getBitstream().getFifo());
      extractFile.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
      extractFile.write(reinterpret_cast<const TChar*>(&(*outputBuffer.begin())), outputAmount);
    }*/
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
    //if (sei->getNumberOfInfoSets() > 0)
    //{
    //  if (m_mctsTidTarget >= nalu.m_temporalId)
    //  {
    //    vector<Int>& idxMCTSBuf = sei->infoSetData(m_mctsEisIdTarget).mctsSetData(m_mctsSetIdxTarget).getMCTSInSet();
    //    if (std::find(idxMCTSBuf.begin(), idxMCTSBuf.end(), currentTileId++) != idxMCTSBuf.end())
    //    {
    //      m_apcSlicePilot->initSlice();
    //      m_apcSlicePilot->setNalUnitType(nalu.m_nalUnitType);
    //      Bool nonReferenceFlag = (m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_TRAIL_N ||
    //        m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_TSA_N ||
    //        m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_STSA_N ||
    //        m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_N ||
    //        m_apcSlicePilot->getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N);
    //      m_apcSlicePilot->setTemporalLayerNonReferenceFlag(nonReferenceFlag);
    //      m_apcSlicePilot->setReferenced(true); // Putting this as true ensures that picture is referenced the first time it is in an RPS
    //      m_apcSlicePilot->setTLayerInfo(nalu.m_temporalId);
    //      m_apcSlicePilot->setNumMCTSTile(sei->infoSetData(m_mctsEisIdTarget).mctsSetData(m_mctsSetIdxTarget).getNumberOfMCTSIdxs());
    //      m_apcSlicePilot->setCountTile(countTile++);
    //      m_cEntropyDecoder.decodeSliceHeader(m_apcSlicePilot, &m_oriParameterSetManager, &m_parameterSetManager, 0);

    //      Int sliceSegmentRsAddress = 0;
    //      if (sei->infoSetData(m_mctsEisIdTarget).m_slice_reordering_enabled_flag)
    //      {
    //        sliceSegmentRsAddress = sei->infoSetData(m_mctsEisIdTarget).outputSliceSegmentAddress(m_apcSlicePilot->getCountTile());
    //      }
    //      else
    //      {
    //        sliceSegmentRsAddress = m_manageSliceAddress.getCtuTsToRsAddrMap((m_extNumCTUs / m_apcSlicePilot->getNumMCTSTile()) * m_apcSlicePilot->getCountTile());
    //      }
    //      m_apcSlicePilot->setSliceSegmentRsAddress(sliceSegmentRsAddress);

    //      writeSlice(mergedFile, nalu, m_apcSlicePilot);
    //    }
    //    if (currentTileId == numTiles)
    //    {
    //      currentTileId = 0;
    //      countTile = 0;
    //    }
    //  }
    //}
  }
  break;
  default:
    break;
  }
}

TComVPS* NalStream::getVPS()
{ 
  while (!mParameterSetManager.getFirstVPS())
  {
    readNALUnit();
  }
  return mParameterSetManager.getFirstVPS();
}

TComSPS* NalStream::getSPS()
{
  while (!mParameterSetManager.getFirstSPS())
  {
    readNALUnit();
  }
  return mParameterSetManager.getFirstSPS();
}

TComPPS* NalStream::getPPS()
{
  while (!mParameterSetManager.getFirstPPS())
  {
    readNALUnit();
  }
  return mParameterSetManager.getFirstPPS();
}