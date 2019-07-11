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

// ====================================================================================================================
// Constructor / destructor / initialization / destroy
// ====================================================================================================================

TAppDecTop::TAppDecTop()
: m_seiReader()
 ,m_pSEIOutputStream(NULL)
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
  ParameterSetManager* parameterSetManager = new ParameterSetManager();
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

  std::vector<Int> tileWidths;
  tileWidths.push_back(768);
  tileWidths.push_back(768);
  tileWidths.push_back(768);
  tileWidths.push_back(768);
  tileWidths.push_back(768);

  std::vector<Int> tileHeights;
  tileHeights.push_back(640);
  tileHeights.push_back(640);
  tileHeights.push_back(640);

  std::vector<Int> tileAddresses;
  tileAddresses.push_back(0);
  tileAddresses.push_back(12);
  tileAddresses.push_back(24);
  tileAddresses.push_back(36);
  tileAddresses.push_back(48);
  tileAddresses.push_back(600);
  tileAddresses.push_back(612);
  tileAddresses.push_back(624);
  tileAddresses.push_back(636);
  tileAddresses.push_back(648);
  tileAddresses.push_back(1200);
  tileAddresses.push_back(1212);
  tileAddresses.push_back(1224);
  tileAddresses.push_back(1236);
  tileAddresses.push_back(1248);

  TComVPS* inVPS = new TComVPS();
  TComSPS* inSPS = new TComSPS();
  TComPPS* inPPS = new TComPPS();

  for (Int iTile = 0; iTile < m_numberOfTiles; iTile++)
  {
    m_pNalStreams[iTile].setTileId(iTile);
    m_pNalStreams[iTile].setOutputStream(mergedFile);
    m_pNalStreams[iTile].setParameterPointers(inVPS, inSPS, inPPS);
    m_pNalStreams[iTile].setTileResolution(tileAddresses, tileWidths, tileHeights);
    m_pNalStreams[iTile].setParameterSetManager(parameterSetManager);
  }

  clock_t start, end;
  double result;

  start = clock(); // 시간 측정 시작

  xInitLogSEI();

  Int frame = 0;
  while (mergedFile.is_open() && frame++ != 299)
  {
    for (Int iTile = 0; iTile < m_numberOfTiles; iTile++)
    {
      m_pNalStreams[iTile].readNALUnit();
    }
  }

  end = clock(); //시간 측정 끝
  result = (double)(end - start);
  printf("%d", static_cast<int>(result));

  if (inVPS)
  {
    delete inVPS;
  }
  if (inPPS)
  {
    delete inPPS;
  }
  if (inSPS)
  {
    delete inSPS;
  }
  if (parameterSetManager)
  {
    delete parameterSetManager;
  }

  mergedFile.close();
  mergedFile.clear();
}

// ====================================================================================================================
// Protected member functions
// ====================================================================================================================

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

Void TAppDecTop::xInitLogSEI()
{
  if (!m_outputDecodedSEIMessagesFilename.empty())
  {
    std::ostream &os=m_seiMessageFileStream.is_open() ? m_seiMessageFileStream : std::cout;
		setSEIMessageOutputStream(&os);
  }
}


