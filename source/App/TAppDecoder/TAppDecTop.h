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

/** \file     TAppDecTop.h
    \brief    Decoder application class (header)
*/

#ifndef __TAPPDECTOP__
#define __TAPPDECTOP__


#pragma once


#include "TAppDecCfg.h"
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

#include "NalStream.h"

//! \ingroup TAppDecoder
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// decoder application class
class TAppDecTop : public TAppDecCfg
{
private:
  
	//editJW
	SEIReader												m_seiReader;
	std::ostream									 *m_pSEIOutputStream;
	TDecEntropy											m_cEntropyDecoder;
	TEncEntropy											m_cEntropyCoder;
	TDecCavlc												m_cCavlcDecoder;
	TEncCavlc												m_cCavlcCoder;
	ParameterSetManager							m_oriParameterSetManager;
	ParameterSetManager							m_parameterSetManager;
	SliceAddressTsRsOrder						m_manageSliceAddress;
	Int															m_extSPSId;
	Int															m_extPPSId;
	Int															m_extNumCTUs;
	
  std::ofstream                   m_seiMessageFileStream;         ///< Used for outputing SEI messages.

  NalStream * m_pNal;

public:
  TAppDecTop();
  virtual ~TAppDecTop() {}

  Void  create            (); ///< create internal members
  Void  destroy           (); ///< destroy internal members
  Void  merge             (); ///< main decoding function

	//edit JW
	Void  setSEIMessageOutputStream(std::ostream *pOpStream) { m_pSEIOutputStream = pOpStream; }

protected:
  Void  xInitDecLib       (); ///< initialize decoder class


private:
	//edit JW
	std::size_t addEmulationPreventionByte(vector<uint8_t>& outputBuffer, vector<uint8_t>& rbsp);
	Void writeParameter(fstream& out, NalUnitType nalUnitType, UInt temporalId, UInt nuhLayerId, vector<uint8_t>& rbsp, ParameterSetManager& parameterSetmanager);
  Void replaceParameter(fstream& out, SEIMCTSExtractionInfoSets& sei, Int mctsEisIdTarget, Int mctsSetIdxTarget, ParameterSetManager& parameterSetmanager);
	Void writeSlice(fstream& out, InputNALUnit& nalu, TComSlice* pcSlice);

  //edit DM
  Void writeVPSSPSPPS(fstream& out, TComVPS* vps, TComSPS* sps, TComPPS* pps);
};

//! \}

#endif

