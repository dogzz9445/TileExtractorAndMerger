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

#include <list>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TAppDecTop.h"
#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"

#include "NalStream.h"

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
 ,m_cCavlcDecoder() 
 ,m_cCavlcCoder()
 ,m_cEntropyDecoder()
 ,m_cEntropyCoder()
 ,m_parameterSetManager()
 ,m_oriParameterSetManager()
 ,m_apcSlicePilot(NULL)
 ,m_manageSliceAddress()
 ,m_extSPSId()
 ,m_extPPSId()
{
}

Void TAppDecTop::create()
{
	m_apcSlicePilot = new TComSlice;

}

Void TAppDecTop::destroy()
{
  m_bitstreamFileName.clear();
	delete m_apcSlicePilot;
	m_apcSlicePilot = NULL;
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
Void TAppDecTop::decode()
{
  std::vector<NalStream*> vecNal;
  std::vector<ifstream*> vecIFStream;
  for (int i = 0; i < 1; i++)
  {
    ifstream* bitstreamFile = new ifstream(m_bitstreamFileName.c_str(), ifstream::in | ifstream::binary);

    if (!*bitstreamFile)
    {
      fprintf(stderr, "\nfailed to open bitstream file `%s' for reading\n", m_bitstreamFileName.c_str());
      exit(EXIT_FAILURE);
    }
    NalStream *nalstream = new NalStream(*bitstreamFile);

    vecNal.push_back(nalstream);
    vecIFStream.push_back(bitstreamFile);
  }

  xInitDecLib  ();

	Int					numCTUs;
	Int					numTiles;
	Int					currentTileId						= 0;
	Int					countTile								= 0;
	Int					bitsSliceSegmentAddress = 0;
	SEIMCTSExtractionInfoSets *sei			= new SEIMCTSExtractionInfoSets;

  std::cout << "1:\n";
  vecNal.at(0)->readNALUnit();
  std::cout << "2:\n";
  vecNal.at(0)->readNALUnit();
  std::cout << "3:\n";
  vecNal.at(0)->readNALUnit();
  std::cout << "4:\n";
  vecNal.at(0)->readNALUnit();
  std::cout << "5:\n";
  vecNal.at(0)->readNALUnit();


  for (std::vector<ifstream*>::iterator it = vecIFStream.begin(); it != vecIFStream.end(); ++it)
  {
    ifstream* stream = *it;

    if (stream)
    {
      (*stream).close();
      delete stream;
    }
  }

  for (std::vector<NalStream*>::iterator it = vecNal.begin(); it != vecNal.end(); ++it)
  {
    NalStream* stream = *it;

    if (stream)
    {
      delete stream;
    }
  }
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================


Void TAppDecTop::replaceParameter(fstream& out, SEIMCTSExtractionInfoSets& sei, Int mctsEisIdTarget, Int mctsSetIdxTarget, ParameterSetManager& parameterSetmanager)
{
  // getNumberOf~ Parametersets for application
	for (Int j = 0; j < sei.infoSetData(mctsEisIdTarget).getNumberOfVPSInInfoSets(); j++)
	{
		out.write(reinterpret_cast<const TChar*>(start_code_prefix), 4);
		vector<uint8_t>& vpsRBSPBuf = sei.infoSetData(mctsEisIdTarget).vpsInInfoSetData(j).getRBSP();
		writeParameter(out, NAL_UNIT_VPS, 0, 0, vpsRBSPBuf, parameterSetmanager);
	}
  out.write(reinterpret_cast<const TChar*>(start_code_prefix), 4);
  vector<uint8_t>& spsRBSPBuf = sei.infoSetData(mctsEisIdTarget).spsInInfoSetData(mctsSetIdxTarget).getRBSP();
  writeParameter(out, NAL_UNIT_SPS, 0, 0, spsRBSPBuf, parameterSetmanager);
	for (Int j = 0; j < sei.infoSetData(mctsEisIdTarget).getNumberOfPPSInInfoSets(); j++)
	{
		out.write(reinterpret_cast<const TChar*>(start_code_prefix), 4);
		vector<uint8_t>& ppsRBSPBuf = sei.infoSetData(mctsEisIdTarget).ppsInInfoSetData(j).getRBSP();
		writeParameter(out, NAL_UNIT_PPS, 0, sei.infoSetData(mctsEisIdTarget).ppsInInfoSetData(j).m_nuh_temporal_id, ppsRBSPBuf, parameterSetmanager);
	}
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

	bsNALUHeader.write(0, 1);                    // forbidden_zero_bit
	bsNALUHeader.write(nalUnitType, 6);  // nal_unit_type
	bsNALUHeader.write(nuhLayerId, 6);   // nuh_layer_id
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

Void TAppDecTop::writeSlice(fstream& out, InputNALUnit& nalu, TComSlice* pcSlice)
{

	if (pcSlice->getSliceType() == I_SLICE)
	{
		out.write(reinterpret_cast<const TChar*>(start_code_prefix + 1), 3);
	}
	else
	{
		if (pcSlice->getCountTile() == 0)
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
	bsNALUHeader.write(nalu.m_nalUnitType, 6);  // nal_unit_type
	bsNALUHeader.write(nalu.m_nuhLayerId, 6);   // nuh_layer_id
	bsNALUHeader.write(nalu.m_temporalId + 1, 3); // nuh_temporal_id_plus1

	out.write(reinterpret_cast<const TChar*>(bsNALUHeader.getByteStream()), bsNALUHeader.getByteStreamLength());

	TComOutputBitstream bsSliceHeader;
	m_cEntropyCoder.setEntropyCoder(&m_cCavlcCoder);
	m_cEntropyCoder.setBitstream(&bsSliceHeader);
	m_cEntropyCoder.encodeSliceHeader(pcSlice);
	bsSliceHeader.writeByteAlignment();

	vector<uint8_t> outputSliceHeaderBuffer;
	std::size_t outputSliceHeaderAmount = 0;
	outputSliceHeaderAmount = addEmulationPreventionByte(outputSliceHeaderBuffer, bsSliceHeader.getFIFO());
	out.write(reinterpret_cast<const TChar*>(&(*outputSliceHeaderBuffer.begin())), outputSliceHeaderAmount);

	TComInputBitstream **ppcSubstreams = NULL;
	TComInputBitstream* pcBitstream = &(nalu.getBitstream());
	const UInt uiNumSubstreams = pcSlice->getNumberOfSubstreamSizes() + 1;

	// init each couple {EntropyDecoder, Substream}
	ppcSubstreams = new TComInputBitstream*[uiNumSubstreams];
	for (UInt ui = 0; ui < uiNumSubstreams; ui++)
	{
		ppcSubstreams[ui] = pcBitstream->extractSubstream(ui + 1 < uiNumSubstreams ? (pcSlice->getSubstreamSize(ui) << 3) : pcBitstream->getNumBitsLeft());
	}
	vector<uint8_t>& sliceRbspBuf = ppcSubstreams[0]->getFifo();

	vector<uint8_t> outputSliceRbspBuffer;
	std::size_t outputRbspHeaderAmount = 0;
	outputRbspHeaderAmount = addEmulationPreventionByte(outputSliceRbspBuffer, sliceRbspBuf);
	out.write(reinterpret_cast<const TChar*>(&(*outputSliceRbspBuffer.begin())), outputRbspHeaderAmount);

}
Void TAppDecTop::xInitDecLib()
{

  if (!m_outputDecodedSEIMessagesFilename.empty())
  {
    std::ostream &os=m_seiMessageFileStream.is_open() ? m_seiMessageFileStream : std::cout;
		setSEIMessageOutputStream(&os);
  }
}


