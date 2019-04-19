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
#include <fstream>

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
 ,m_manageSliceAddress()
 ,m_extSPSId()
 ,m_extPPSId()
{
}

Void TAppDecTop::create()
{
}

Void TAppDecTop::destroy()
{
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
Void TAppDecTop::merge()
{
  std::ifstream mStream(m_inBitstreamFileNames, std::ifstream::in | std::ifstream::binary);

  if (!mStream.is_open())
  {
    std::cerr << "Warning: File is not opened\n";
    exit(EXIT_FAILURE);
  }

  InputByteStream mByteStream(mStream);

  xInitDecLib(); 
  xInitLogSEI();

  TDecEntropy mEntropyDecoder = TDecEntropy();
  TDecCavlc mCavlcDecoder = TDecCavlc();
  ParameterSetManager mParameterSetManager = ParameterSetManager();
  AnnexBStats mStats = AnnexBStats();

  mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);

  while (mStream)
  {
    InputNALUnit nalu;
    byteStreamNALUnit(mByteStream, nalu.getBitstream().getFifo(), mStats);

    read(nalu);
    if (nalu.getBitstream().getFifo().empty())
    {
      std::cerr << "Waring: Attempt to decode an empty NAL unit\n";
    }

    mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);
    mEntropyDecoder.setBitstream(&(nalu.getBitstream()));

    std::cout << "NAL_TYPE: " << nalUnitTypeToString(nalu.m_nalUnitType) << std::endl;
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
      xWriteBitstream(nalu, mParameterSetManager);
    }
    break;
    default:
      break;
    }
  } 
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

Void TAppDecTop::xWriteBitstream(InputNALUnit& nalUnit, ParameterSetManager& para)
{
  TDecEntropy mEntropyDecoder = TDecEntropy();
  TDecCavlc mCavlcDecoder = TDecCavlc();

  mEntropyDecoder.setEntropyDecoder(&mCavlcDecoder);
  mEntropyDecoder.setBitstream(&nalUnit.getBitstream());

  TComSlice slice;
  slice.initSlice();
  slice.setNalUnitType(nalUnit.m_nalUnitType);

  mEntropyDecoder.decodeSliceHeader(&slice, &para, &para, 0);
  
  std::cout << "read bits: " << nalUnit.getBitstream().getNumBitsRead() << std::endl
    << "left bits: " << nalUnit.getBitstream().getNumBitsLeft() << std::endl;
  
  //Int EntireWidth = 512;
  //Int EntireHeight = 320;

  //// slice.getSPS()->setPicWidthInLumaSamples(EntireWidth);
  //// slice.getSPS()->setPicHeightInLumaSamples(EntireHeight);


  //if (tileId == 0)
  //{
  //  slice.setSliceSegmentRsAddress(0);
  //}
  //else if (tileId == 1)
  //{
  //  slice.setSliceSegmentRsAddress(4);
  //}
  //else if (tileId == 2)
  //{
  //  slice.setSliceSegmentRsAddress(16);
  //}
  //else if (tileId == 3)
  //{
  //  slice.setSliceSegmentRsAddress(20);
  //}

  //if (slice.getSliceType() == I_SLICE)
  //{
  //  out.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
  //}
  //else
  //{
  //  if (slice.getCountTile() == 0)
  //  {
  //    out.write(reinterpret_cast<const TChar*>(start_code_prefix), 4);
  //  }
  //  else
  //  {
  //    out.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
  //  }
  //}

  //TComOutputBitstream bsNALUHeader;

  //bsNALUHeader.write(0, 1);                    // forbidden_zero_bit
  //bsNALUHeader.write(inNal.m_nalUnitType, 6);  // nal_unit_type
  //bsNALUHeader.write(inNal.m_nuhLayerId, 6);   // nuh_layer_id
  //bsNALUHeader.write(inNal.m_temporalId + 1, 3); // nuh_temporal_id_plus1

  //std::cout << nalUnitTypeToString(inNal.m_nalUnitType) << ": " << bsNALUHeader.getByteStreamLength() << std::endl;
  //out.write(reinterpret_cast<const TChar*>(bsNALUHeader.getByteStream()), bsNALUHeader.getByteStreamLength());

  //TComOutputBitstream bsSliceHeader;
  //m_cEntropyCoder.setEntropyCoder(&m_cCavlcCoder);
  //m_cEntropyCoder.setBitstream(&bsSliceHeader);
  //m_cEntropyCoder.encodeSliceHeader(&slice);
  //bsSliceHeader.writeByteAlignment();

  //vector<uint8_t> outputSliceHeaderBuffer;
  //std::size_t outputSliceHeaderAmount = 0;
  //outputSliceHeaderAmount = addEmulationPreventionByte(outputSliceHeaderBuffer, bsSliceHeader.getFIFO());
  //// bsSliceHeader에 add해서 처리할 수 있도록

  //out.write(reinterpret_cast<const TChar*>(&(*outputSliceHeaderBuffer.begin())), outputSliceHeaderAmount);

  //TComInputBitstream** ppcSubstreams = NULL;
  //TComInputBitstream*  pcBitstream = &(inNal.getBitstream());
  //const UInt uiNumSubstreams = slice.getNumberOfSubstreamSizes() + 1;

  //// init each couple {EntropyDecoder, Substream}
  //ppcSubstreams = new TComInputBitstream*[uiNumSubstreams];
  //for (UInt ui = 0; ui < uiNumSubstreams; ui++)
  //{
  //  ppcSubstreams[ui] = pcBitstream->extractSubstream(ui + 1 < uiNumSubstreams ? (slice.getSubstreamSize(ui) << 3) : pcBitstream->getNumBitsLeft());
  //  //ppcSubstreams[ui] = pcBitstream->extractSubstream(slice.getSubstreamSize(ui) << 3);
  //}
  //vector<uint8_t>& sliceRbspBuf = ppcSubstreams[0]->getFifo();

  //vector<uint8_t> outputSliceRbspBuffer;
  //std::size_t outputRbspHeaderAmount = 0;
  //outputRbspHeaderAmount = addEmulationPreventionByte(outputSliceRbspBuffer, sliceRbspBuf);

  //out.write(reinterpret_cast<const TChar*>(&(*outputSliceRbspBuffer.begin())), outputRbspHeaderAmount);
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


Void TAppDecTop::xInitDecLib()
{
}

Void TAppDecTop::xInitLogSEI()
{
}


