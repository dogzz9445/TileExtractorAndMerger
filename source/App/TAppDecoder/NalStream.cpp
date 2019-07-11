#include "NalStream.h"
#include "TilePartition.h"

static const UChar start_code[] = { 0, 0, 0, 1 };

NalStream::NalStream()
: mEntropyDecoder(TDecEntropy()),
  mCavlcDecoder(TDecCavlc()),
  mEntropyCoder(TEncEntropy()),
  mCavlcEncoder(TEncCavlc()),
  mStats(AnnexBStats())
{
  mEntropyCoder.setEntropyCoder(&mCavlcEncoder);
  mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);
  emVPS = new TComVPS();
  emSPS = new TComSPS();
  emPPS = new TComPPS();
  mOriParameterSetManager = new ParameterSetManager();
}

NalStream::NalStream(const char* filename) 
  : mEntropyDecoder(TDecEntropy()),
  mCavlcDecoder(TDecCavlc()),
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

Void NalStream::setParameterPointers(TComVPS* vps, TComSPS* sps, TComPPS* pps)
{
  inVPS = vps;
  inSPS = sps;
  inPPS = pps;
}

Void NalStream::setTileResolution(std::vector<Int>& tileAddresses, std::vector<Int>& tileWidths, std::vector<Int>& tileHeights)
{
  mTileWidths = tileWidths;
  mTileHeights = tileHeights;
  mTileAddresses = tileAddresses;
}

Void NalStream::readNALUnit()
{
  nalu = InputNALUnit();

  byteStreamNALUnit(*mByteStream, nalu.getBitstream().getFifo(), mStats);

  read(nalu);
  std::cout << nalu.m_nalUnitType << std::endl;
  std::cout << "Nalu FIFO: " << nalu.getBitstream().getFifo().size() << std::endl;
  std::cout << "Read Bits: " << nalu.getBitstream().getNumBitsRead() << std::endl;
  std::cout << "Left Bits: " << nalu.getBitstream().getNumBitsLeft() << std::endl;

  if (nalu.getBitstream().getFifo().empty())
  {
    std::cerr << "Waring: Attempt to decode an empty NAL unit\n";
  }
  mEntropyDecoder.setBitstream(&(nalu.getBitstream()));
    
  switch (nalu.m_nalUnitType)
  {
  case NAL_UNIT_VPS:
  {
    emVPS = new TComVPS();
    mEntropyDecoder.decodeVPS(emVPS);
    mOriParameterSetManager->storeVPS(emVPS, nalu.getBitstream().getFifo());

    if (mTileId == 0)
    {
      *inVPS = *emVPS;

      mExtVPSId = inVPS->getVPSId();

      OutputNALUnit outVPS(NAL_UNIT_VPS);
      mEntropyCoder.setBitstream(&(outVPS.m_Bitstream));
      mEntropyCoder.encodeVPS(inVPS);
      accessUnit.push_back(new NALUnitEBSP(outVPS));
      const vector<UInt>& statsTop = writeAnnexB(*outStream, accessUnit);
      accessUnit.clear();

      mParameterSetManager->storeVPS(inVPS, outVPS.m_Bitstream.getFIFO());
    }
  }
  break;
  case NAL_UNIT_SPS:
  {
    emSPS = new TComSPS();
    mEntropyDecoder.decodeSPS(emSPS);
    mOriParameterSetManager->storeSPS(emSPS, nalu.getBitstream().getFifo());

    if (mTileId == 0)
    {
      *inSPS = *emSPS;

      inSPS->setPicWidthInLumaSamples(ENTIRE_WIDTH);
      inSPS->setPicHeightInLumaSamples(ENTIRE_HEIGHT);

      mExtSPSId = inSPS->getSPSId();

      OutputNALUnit outSPS(NAL_UNIT_SPS);
      mEntropyCoder.setBitstream(&(outSPS.m_Bitstream));
      mEntropyCoder.encodeSPS(inSPS);
      accessUnit.push_back(new NALUnitEBSP(outSPS));
      const vector<UInt>& statsTop = writeAnnexB(*outStream, accessUnit);
      accessUnit.clear();

      mParameterSetManager->storeSPS(inSPS, outSPS.m_Bitstream.getFIFO());

    }
  }
  break;
  case NAL_UNIT_PPS:
  {
    emPPS = new TComPPS();
    mEntropyDecoder.decodePPS(emPPS);
    mOriParameterSetManager->storePPS(emPPS, nalu.getBitstream().getFifo());

    if (mTileId == 0)
    {
      *inPPS = *emPPS;

      inPPS->setLoopFilterAcrossSlicesEnabledFlag(false);
      inPPS->setLoopFilterAcrossTilesEnabledFlag(true);
      inPPS->setTilesEnabledFlag(true);
      inPPS->setTileUniformSpacingFlag(true);
      inPPS->setTileColumnWidth(mTileWidths);
      inPPS->setTileRowHeight(mTileHeights);
      inPPS->setNumTileColumnsMinus1(mTileWidths.size() - 1);
      inPPS->setNumTileRowsMinus1(mTileHeights.size() - 1);

      mExtPPSId = inPPS->getPPSId();

      OutputNALUnit outPPS(NAL_UNIT_PPS);
      mEntropyCoder.setBitstream(&(outPPS.m_Bitstream));
      mEntropyCoder.encodePPS(inPPS);
      accessUnit.push_back(new NALUnitEBSP(outPPS));
      const vector<UInt>& statsTop = writeAnnexB(*outStream, accessUnit);
      accessUnit.clear();

      mParameterSetManager->storePPS(inPPS, outPPS.m_Bitstream.getFIFO());
    }
  }  
  break;
  case NAL_UNIT_PREFIX_SEI:
  {
    if (mTileId == 0)
    {
      vector<uint8_t> outputBuffer;
      std::size_t outputAmount = 0;
      outputAmount = addEmulationPreventionByte(outputBuffer, nalu.getBitstream().getFifo());
      outStream->write(reinterpret_cast<const TChar*>(start_code + 1), 3);
      outStream->write(reinterpret_cast<const TChar*>(&(*outputBuffer.begin())), outputAmount);
    }
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
    TComSlice slice;
    slice.initSlice();
    slice.setNalUnitType(nalu.m_nalUnitType);

    mEntropyDecoder.decodeSliceHeader(&slice, mOriParameterSetManager, 0);

    if (slice.getSliceType() == I_SLICE)
    {
      outStream->write(reinterpret_cast<const TChar*>(start_code + 1), 3);
    }
    else
    {
      if (mTileId == 0)
      {
        outStream->write(reinterpret_cast<const TChar*>(start_code), 4);
      }
      else
      {
        outStream->write(reinterpret_cast<const TChar*>(start_code + 1), 3);
      }
    }

    slice.setPPS(inPPS);
    slice.setSPS(inSPS);
    slice.setSliceSegmentRsAddress(mTileAddresses.at(mTileId));
    
    TComOutputBitstream bsNALUHeader;
    bsNALUHeader.write(0, 1);                    // forbidden_zero_bit
    bsNALUHeader.write(nalu.m_nalUnitType, 6);  // nal_unit_type
    bsNALUHeader.write(nalu.m_nuhLayerId, 6);   // nuh_layer_id
    bsNALUHeader.write(nalu.m_temporalId + 1, 3); // nuh_temporal_id_plus1

    std::cout << "Read Bits: " << nalu.getBitstream().getNumBitsRead() << std::endl;
    std::cout << "Left Bits: " << nalu.getBitstream().getNumBitsLeft() << std::endl;
    std::cout << "NAL Header Size: " << bsNALUHeader.getByteStreamLength() << std::endl;
    outStream->write(reinterpret_cast<const TChar*>(bsNALUHeader.getByteStream()), bsNALUHeader.getByteStreamLength());

    TComOutputBitstream bsSliceHeader;
    mEntropyCoder.setBitstream(&bsSliceHeader);
    mEntropyCoder.encodeSliceHeader(&slice);
    bsSliceHeader.writeByteAlignment();

    vector<uint8_t> outputSliceHeaderBuffer;
    std::size_t outputSliceHeaderAmount = 0;
    outputSliceHeaderAmount = addEmulationPreventionByte(outputSliceHeaderBuffer, bsSliceHeader.getFIFO());
    // bsSliceHeader에 add해서 처리할 수 있도록

    std::cout << "Slice Header Size: " << outputSliceHeaderAmount << std::endl;
    outStream->write(reinterpret_cast<const TChar*>(&(*outputSliceHeaderBuffer.begin())), outputSliceHeaderAmount);
    
    TComInputBitstream** ppcSubstreams = NULL;
    TComInputBitstream*  pcBitstream = &(nalu.getBitstream());
    const UInt uiNumSubstreams = slice.getNumberOfSubstreamSizes() + 1;

    // init each couple {EntropyDecoder, Substream}
    ppcSubstreams = new TComInputBitstream*[uiNumSubstreams];
    for (UInt ui = 0; ui < uiNumSubstreams; ui++)
    {
      Int a = pcBitstream->getNumBitsLeft();
      ppcSubstreams[ui] = pcBitstream->extractSubstream(ui + 1 < uiNumSubstreams ? (slice.getSubstreamSize(ui) << 3) : pcBitstream->getNumBitsLeft());
      //ppcSubstreams[ui] = pcBitstream->extractSubstream(slice.getSubstreamSize(ui) << 3);
    }
    vector<uint8_t>& sliceRbspBuf = ppcSubstreams[0]->getFifo();

    vector<uint8_t> outputSliceRbspBuffer;
    std::size_t outputRbspHeaderAmount = 0;
    outputRbspHeaderAmount = addEmulationPreventionByte(outputSliceRbspBuffer, sliceRbspBuf);

    std::cout << "Data Size: " << outputRbspHeaderAmount << std::endl;
    outStream->write(reinterpret_cast<const TChar*>(&(*outputSliceRbspBuffer.begin())), outputRbspHeaderAmount);
  }
  break;
  default:
    break;
  }
}

std::size_t addEmulationPreventionByte(vector<uint8_t>& outputBuffer, vector<uint8_t>& rbsp)
{
  outputBuffer.resize(rbsp.size() * 2 + 1); //there can never be enough emulation_prevention_three_bytes to require this much space
  std::size_t outputAmount = 0;
  Int         zeroCount = 0;
  for (vector<uint8_t>::iterator it = rbsp.begin(); it != rbsp.end(); it++)
  {
    const uint8_t v = (*it);
    if (zeroCount == 2 && v <= 3)
    {
      outputBuffer[outputAmount++] = emulation_prevention_three_byte[0];
      zeroCount = 0;
    }

    if (v == 0)
    {
      zeroCount++;
    }
    else
    {
      zeroCount = 0;
    }
    outputBuffer[outputAmount++] = v;
  }

  /* 7.4.1.1
  * ... when the last byte of the RBSP data is equal to 0x00 (which can
  * only occur when the RBSP ends in a cabac_zero_word), a final byte equal
  * to 0x03 is appended to the end of the data.
  */
  if (zeroCount>0)
  {
    outputBuffer[outputAmount++] = emulation_prevention_three_byte[0];
  }

  return outputAmount;
}