/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TAppDecTop.cpp
    \brief    Decoder application class
*/

#define DM_TEST

#include <list>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppDecTop.h"

//! \ingroup TAppDecoder
//! \{

static const UChar emulation_prevention_three_byte[] = { 3 };
static const UChar start_code_prefix[] = { 0, 0, 0, 1 };

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppDecTop::TAppDecTop()
: m_seiReader()
 ,m_pSEIOutputStream(NULL)
 ,m_cCavlcCoder()
 ,m_cCavlcDecoder()
 ,m_cEntropyCoder()
 ,m_cEntropyDecoder()
 ,m_parameterSetManager()
 ,m_oriParameterSetManager()
 ,m_manageSliceAddress()
 ,m_extSPSId()
 ,m_extPPSId()
 ,m_pNalStreams(NULL)
 ,m_tilePartitionManager()
{
}

Void TAppDecTop::create()
{
}

Void TAppDecTop::destroy()
{
  m_outBitstreamFileName.clear();

  if (m_pNalStreams)
  {
    delete[] m_pNalStreams;
    m_pNalStreams = NULL;
  }
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/**
 - create internal class
 - initialize internal class
 - until the end of the bitstream, call decoding function in TDecTop class
 - delete allocated buffers
 - destroy internal class
 .
 */
Void TAppDecTop::merge(Int sizeGOP)
{
  m_pNalStreams = new NalStream[m_numberOfTiles];
  for (int i = 0; i < m_numberOfTiles; i++)
  {
    m_pNalStreams[i].addFile(m_inBitstreamFileNames[i].c_str());
  }

  fstream mergedFile(m_outBitstreamFileName, fstream::binary | fstream::out);
  if (!mergedFile)
  {
    std::cerr << "\nfailed to open bitstream file for writing\n";
    exit(EXIT_FAILURE);
  }

  m_tilePartitionManager.setNumTiles(m_numberOfTiles);
  m_tilePartitionManager.setNumTilesInColumn(m_numberOfTilesInColumn);
  m_tilePartitionManager.setNumTilesInRow(m_numberOfTilesInRow);
  m_tilePartitionManager.setEntireWidth(m_iTargetWidth);
  m_tilePartitionManager.setEntireHeight(m_iTargetHeight);
  m_tilePartitionManager.setTileUniformSpacingFlag(m_tileUniformFlag);

  if (!m_tileUniformFlag)
  {
    std::vector<Int> tileWidths;
    std::vector<Int> tileHeights;
    //m_tilePartitionManager.setTileWidths();
    //m_tilePartitionManager.setTileHeights();
  }

  xInitLogSEI();

  m_cEntropyCoder.setEntropyCoder(&m_cCavlcCoder);
  m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);

  if (mergedFile.is_open())
  {
    // VPS SPS, PPS 정보 파싱
    for (Int iVPSSPSPPS = 0; iVPSSPSPPS < 3; iVPSSPSPPS++)
    {
      for (Int iTile = 0; iTile < m_numberOfTiles; iTile++)
      {
        InputNALUnit nalu;
        m_pNalStreams[iTile].readNALUnit(nalu);
      }
    }

    std::cout << "Writing VPS SPS PPS...\n";
    InputNALUnit nalu_ivps;
    TComVPS* inVps = m_pNalStreams[0].getVPS(nalu_ivps);
    InputNALUnit nalu_isps;
    TComSPS* inSps = m_pNalStreams[0].getSPS(nalu_isps);
    InputNALUnit nalu_ipps;
    TComPPS* inPps = m_pNalStreams[0].getPPS(nalu_ipps);

    inSps->setPicWidthInLumaSamples(m_iTargetWidth);
    inSps->setPicHeightInLumaSamples(m_iTargetHeight);

    std::vector<Int> tileWidths;
    tileWidths.push_back(768);
    tileWidths.push_back(768);
    tileWidths.push_back(768);
    tileWidths.push_back(768);
    tileWidths.push_back(768);

    std::vector<Int> tileHeigths;
    tileHeigths.push_back(640);
    tileHeigths.push_back(640);
    tileHeigths.push_back(640);

    inPps->setLoopFilterAcrossSlicesEnabledFlag(false);
    inPps->setLoopFilterAcrossTilesEnabledFlag(true);
    inPps->setTilesEnabledFlag(true);
    inPps->setTileUniformSpacingFlag(true);
    inPps->setTileColumnWidth(tileWidths);
    inPps->setTileRowHeight(tileHeigths);
    inPps->setNumTileColumnsMinus1(m_numberOfTilesInColumn - 1);
    inPps->setNumTileRowsMinus1(m_numberOfTilesInRow - 1);

    xWriteVPSSPSPPS(mergedFile, inVps, inSps, inPps);

    for (Int iFrame = 0; iFrame < /*max frame*/32; iFrame++)
    {
      AccessUnit accessUnit;
      for (Int iTile = 0; iTile < m_numberOfTiles; iTile++)
      {
        std::cout << "\n";
        InputNALUnit nalu = InputNALUnit();
        m_pNalStreams[iTile].getSliceNAL(nalu);

        while (nalu.m_nalUnitType == NAL_UNIT_PREFIX_SEI)
        {
          if (iTile == 0)
          {
            vector<uint8_t> outputBuffer;
            std::size_t outputAmount = 0;
            outputAmount = addEmulationPreventionByte(outputBuffer, nalu.getBitstream().getFifo());
            mergedFile.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
            mergedFile.write(reinterpret_cast<const TChar*>(&(*outputBuffer.begin())), outputAmount);
          }
          nalu = InputNALUnit();
          m_pNalStreams[iTile].getSliceNAL(nalu);
        }

        xWriteBitstream(mergedFile, accessUnit, nalu, &m_pNalStreams[iTile], iTile);
      }
    } 
  } 

  mergedFile.close();
  mergedFile.clear();
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TAppDecTop::xWriteVPSSPSPPS(std::ostream& out, const TComVPS* vps, const TComSPS* sps, const TComPPS* pps)
{
  AccessUnit accessUnit;
  xWriteVPS(accessUnit, vps);
  xWriteSPS(accessUnit, sps);
  xWritePPS(accessUnit, pps);

  const vector<UInt>& statsTop = writeAnnexB(out, accessUnit);
}

Void TAppDecTop::xWriteVPS(AccessUnit &accessUnit, const TComVPS* vps)
{
  OutputNALUnit nalu_VPS(NAL_UNIT_VPS);
  m_cEntropyCoder.setBitstream(&(nalu_VPS.m_Bitstream));
  m_cEntropyCoder.encodeVPS(vps);
  accessUnit.push_back(new NALUnitEBSP(nalu_VPS));
}

Void TAppDecTop::xWriteSPS(AccessUnit &accessUnit, const TComSPS* sps)
{
  OutputNALUnit nalu_SPS(NAL_UNIT_SPS);
  m_cEntropyCoder.setBitstream(&(nalu_SPS.m_Bitstream));
  m_cEntropyCoder.encodeSPS(sps);
  accessUnit.push_back(new NALUnitEBSP(nalu_SPS));
}

Void TAppDecTop::xWritePPS(AccessUnit &accessUnit, const TComPPS* pps)
{
  OutputNALUnit nalu_PPS(NAL_UNIT_PPS);
  m_cEntropyCoder.setBitstream(&(nalu_PPS.m_Bitstream));
  m_cEntropyCoder.encodePPS(pps);
  accessUnit.push_back(new NALUnitEBSP(nalu_PPS));
}

Void TAppDecTop::xWriteBitstream(
  std::ostream& out, 
  AccessUnit&   accessUnit, 
  InputNALUnit& inNal, 
  NalStream*    nalStream, 
  Int&          tileId)
{
  m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream(&inNal.getBitstream());

  TComSlice slice;
  slice.initSlice();
  slice.setNalUnitType(inNal.m_nalUnitType);

  Bool nonReferenceFlag = (
    slice.getNalUnitType() == NAL_UNIT_CODED_SLICE_TRAIL_N ||
    slice.getNalUnitType() == NAL_UNIT_CODED_SLICE_TSA_N ||
    slice.getNalUnitType() == NAL_UNIT_CODED_SLICE_STSA_N ||
    slice.getNalUnitType() == NAL_UNIT_CODED_SLICE_RADL_N ||
    slice.getNalUnitType() == NAL_UNIT_CODED_SLICE_RASL_N
    );
  slice.setTemporalLayerNonReferenceFlag(nonReferenceFlag);
  slice.setReferenced(true); // Putting this as true ensures that picture is referenced the first time it is in an RPS
  slice.setTLayerInfo(inNal.m_temporalId);

  m_cEntropyDecoder.decodeSliceHeader(&slice, nalStream->getParameterSetManager(), &m_parameterSetManager, 0);
  
  Int EntireWidth = 3840;
  Int EntireHeight = 1920;

  if (tileId == 0)
  {
    slice.setSliceSegmentRsAddress(0);
  }
  else if (tileId == 1)
  {
    slice.setSliceSegmentRsAddress(12);
  }
  else if (tileId == 2)
  {
    slice.setSliceSegmentRsAddress(24);
  }
  else if (tileId == 3)
  {
    slice.setSliceSegmentRsAddress(36);
  }
  else if (tileId == 4)
  {
    slice.setSliceSegmentRsAddress(48);
  }
  else if (tileId == 5)
  {
    slice.setSliceSegmentRsAddress(600);
  }
  else if (tileId == 6)
  {
    slice.setSliceSegmentRsAddress(612);
  }
  else if (tileId == 7)
  {
    slice.setSliceSegmentRsAddress(624);
  }
  else if (tileId == 8)
  {
    slice.setSliceSegmentRsAddress(636);
  }
  else if (tileId == 9)
  {
    slice.setSliceSegmentRsAddress(648);
  }
  else if (tileId == 10)
  {
    slice.setSliceSegmentRsAddress(1200);
  }
  else if (tileId == 11)
  {
    slice.setSliceSegmentRsAddress(1212);
  }
  else if (tileId == 12)
  {
    slice.setSliceSegmentRsAddress(1224);
  }
  else if (tileId == 13)
  {
    slice.setSliceSegmentRsAddress(1236);
  }
  else if (tileId == 14)
  {
    slice.setSliceSegmentRsAddress(1248);
  }

  if (slice.getSliceType() == I_SLICE)
  {
    out.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
  }
  else
  {
    if (slice.getCountTile() == 0)
    {
      out.write(reinterpret_cast<const TChar*>(start_code_prefix), 4);
    }
    else
    {
      out.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
    }
  }

  TComOutputBitstream bsNALUHeader;

  bsNALUHeader.write(0, 1);                    // forbidden_zero_bit
  bsNALUHeader.write(inNal.m_nalUnitType, 6);  // nal_unit_type
  bsNALUHeader.write(inNal.m_nuhLayerId, 6);   // nuh_layer_id
  bsNALUHeader.write(inNal.m_temporalId + 1, 3); // nuh_temporal_id_plus1

  out.write(reinterpret_cast<const TChar*>(bsNALUHeader.getByteStream()), bsNALUHeader.getByteStreamLength());

  TComOutputBitstream bsSliceHeader;
  m_cEntropyCoder.setEntropyCoder(&m_cCavlcCoder);
  m_cEntropyCoder.setBitstream(&bsSliceHeader);
  m_cEntropyCoder.encodeSliceHeader(&slice);
  bsSliceHeader.writeByteAlignment();
  std::cout << nalUnitTypeToString(inNal.m_nalUnitType) << ": " << bsSliceHeader.getByteStreamLength() << std::endl;

  vector<uint8_t> outputSliceHeaderBuffer;
  std::size_t outputSliceHeaderAmount = 0;
  outputSliceHeaderAmount = addEmulationPreventionByte(outputSliceHeaderBuffer, bsSliceHeader.getFIFO());
  // bsSliceHeader에 add해서 처리할 수 있도록

  std::cout << "SliceHeaderAmount: " << outputSliceHeaderAmount << std::endl;

  out.write(reinterpret_cast<const TChar*>(&(*outputSliceHeaderBuffer.begin())), outputSliceHeaderAmount);

  TComInputBitstream** ppcSubstreams = NULL;
  TComInputBitstream*  pcBitstream = &(inNal.getBitstream());
  const UInt uiNumSubstreams = slice.getNumberOfSubstreamSizes() + 1;

  std::cout << "uiNumSubstream: " << uiNumSubstreams << std::endl;

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

  std::cout << "SliceRbspAmount: " << outputRbspHeaderAmount << std::endl;

  out.write(reinterpret_cast<const TChar*>(&(*outputSliceRbspBuffer.begin())), outputRbspHeaderAmount);
  //TComInputBitstream* pcBitstream = &(inNal.getBitstream());
  //pcBitstream->extractSubstream();
  //const UInt

  //std::size_t outputRbspHeaderAmout = 0;
  //outputRbspHeaderAmount = addEmulationPreventionByte(outputSliceRbspBuffer, sliceRbspBuf);
  //std::cout << "read bytes: " << pcBitstream->getNumBitsRead() << std::endl
  //  << "left bytes: " << pcBitstream->getNumBitsLeft() << std::endl;
  //out.write(reinterpret_cast<const TChar*>(pcBitstream + (pcBitstream->getNumBitsRead() / 8)), pcBitstream->getNumBitsLeft());
}


std::size_t TAppDecTop::addEmulationPreventionByte(vector<uint8_t>& outputBuffer, vector<uint8_t>& rbsp)
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

Void TAppDecTop::writeParameter(fstream& out, NalUnitType nalUnitType, UInt nuhLayerId, UInt temporalId, vector<uint8_t>& rbsp, ParameterSetManager& parameterSetmanager)
{
	
	TComOutputBitstream bsNALUHeader;

	bsNALUHeader.write(0, 1);              // forbidden_zero_bit
	bsNALUHeader.write(nalUnitType, 6);    // nal_unit_type
	bsNALUHeader.write(nuhLayerId, 6);     // nuh_layer_id
	bsNALUHeader.write(temporalId + 1, 3); // nuh_temporal_id_plus1

	out.write(reinterpret_cast<const TChar*>(bsNALUHeader.getByteStream()), bsNALUHeader.getByteStreamLength());

	vector<uint8_t> outputBuffer;
	std::size_t outputAmount = 0;
	outputAmount = addEmulationPreventionByte(outputBuffer, rbsp);

	out.write(reinterpret_cast<const TChar*>(&(*outputBuffer.begin())), outputAmount);

	
	if (nalUnitType == NAL_UNIT_SPS)
	{
		TComSPS*		sps = new TComSPS();
		InputNALUnit nalu;
		vector<uint8_t>& nalUnitBuf = nalu.getBitstream().getFifo();
		vector<uint8_t>& nalUnitHeaderBuf = bsNALUHeader.getFIFO();
		nalUnitBuf.resize(bsNALUHeader.getByteStreamLength() + outputAmount);
		UChar *NewDataArray = &nalUnitBuf[0];
		UChar *OldHeaderDataArray = &nalUnitHeaderBuf[0];
		memcpy(NewDataArray, OldHeaderDataArray, bsNALUHeader.getByteStreamLength());
		memcpy(NewDataArray + bsNALUHeader.getByteStreamLength(), &outputBuffer[0], outputAmount);
		read(nalu);
    m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);
		m_cEntropyDecoder.setBitstream(&(nalu.getBitstream()));
		m_cEntropyDecoder.decodeSPS(sps);
		m_parameterSetManager.storeSPS(sps, nalu.getBitstream().getFifo());
		m_extSPSId = sps->getSPSId();
		m_extNumCTUs = ((sps->getPicWidthInLumaSamples() + sps->getMaxCUWidth() - 1) / sps->getMaxCUWidth())*((sps->getPicHeightInLumaSamples() + sps->getMaxCUHeight() - 1) / sps->getMaxCUHeight());
	
	}
	else if (nalUnitType == NAL_UNIT_PPS)
	{
		TComPPS*		pps = new TComPPS();
		InputNALUnit nalu;
		vector<uint8_t>& nalUnitBuf = nalu.getBitstream().getFifo();
		vector<uint8_t>& nalUnitHeaderBuf = bsNALUHeader.getFIFO();
		nalUnitBuf.resize(bsNALUHeader.getByteStreamLength() + outputAmount);
		UChar *NewDataArray = &nalUnitBuf[0];
		UChar *OldHeaderDataArray = &nalUnitHeaderBuf[0];
		memcpy(NewDataArray, OldHeaderDataArray, bsNALUHeader.getByteStreamLength());
		memcpy(NewDataArray + bsNALUHeader.getByteStreamLength(), &outputBuffer[0], outputAmount);
		read(nalu);
		m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);
		m_cEntropyDecoder.setBitstream(&(nalu.getBitstream()));
		m_cEntropyDecoder.decodePPS(pps);
		m_parameterSetManager.storePPS(pps, nalu.getBitstream().getFifo());
		m_extPPSId = pps->getPPSId();
	}
}
Void TAppDecTop::xInitLogSEI()
{
  if (!m_outputDecodedSEIMessagesFilename.empty())
  {
    std::ostream &os=m_seiMessageFileStream.is_open() ? m_seiMessageFileStream : std::cout;
		setSEIMessageOutputStream(&os);
  }
}


