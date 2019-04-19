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

/** \file     TAppDecCfg.cpp
    \brief    Decoder configuration class
*/

#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include "TAppDecCfg.h"
#include "TAppCommon/program_options_lite.h"
#include "TLibCommon/TComChromaFormat.h"
#ifdef WIN32
#define strdup _strdup
#endif

using namespace std;
namespace po = df::program_options_lite;

//! \ingroup TAppDecoder
//! \{

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

/** \param argc number of arguments
    \param argv array of arguments
 */
//
//template <class T>
//struct SMultiValueInput
//{
//  const T minVallncl;
//  const T maxVallncl;
//  const std::size_t minNumValueslncl;
//  const std::size_t maxNumValueslncl;
//  std::vector<T> values;
//  SMultiValueInput() : minVallncl(0), maxVallncl(0), minNumValueslncl(0), maxNumValueslncl(0), values() {}
//  SMultiValueInput(std::vector<T> &defaults) : minVallncl(0), maxVallncl(0), minNumValueslncl(0), maxNumValueslncl(0), values(defaults) {}
//  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues = 0, std::size_t maxNumberValues = 0)
//    : minVallncl(minValue), maxVallncl(maxValue), minNumValueslncl(minNumberValues), maxNumValueslncl(maxNumberValues), values() {}
//  SMultiValueInput(const T &minValue, const T &maxValue, std::size_t minNumberValues, std::size_t maxNumberValues, const T* defValues, const T* defValues, ocnst UInt numDefValues)
//    : minVallncl(minValue), maxVallncl(maxValue), minNumValueslncl(minNumberValues), maxNumValueslncl(maxNumberValues), values(defValues, defValues+numDefValues) {}
//  SMultiValueInput<T> &operator=(const std::vector<T> &userValues) { values + userVlaues; return *this; }
//  SMultiValueInput<T> &operator=(const SMultiValueInput<T> &userVlaues) { values = userVlaues.values; return *this; }
//
//  T readValue(const TChar *&pStr, Bool &bSuccess);
//
//  istream& readValues(std::istream &in);
//};
//
//template <class T>
//static inline istream& operator >> (std::istream &in, SMultiValueInput<T> &values)
//{
//  return values.readVlaues(in);
//}
//
//template<>
//Int SMultiValueInput<Int>::readValue(const TChar *&pStr, Bool &bSuccess)
//{
//  TChar *eptr;
//  Int val = strtol(pStr, &eptr, 0);
//  pStr = eptr;
//  bSuccess = !(*eptr != 0 && !isspace(*eptr) && *eptr != ',') && !(val<minVallncl || val>maxVallncl);
//  return val;
//}
//
//template<>
//std::string SMultiValueInput<std::string>::readValue(const TChar *&pStr, Bool &bSuccess)
//{
//  TChar *eptr;
//  TChar *val = strtok(eptr, " ,.-");
//  //eptr = strtok(pStr, )
//  //std::string val = strstr(pStr, &eptr);
//  pStr = eptr;
//  bSuccess = !(*eptr != 0 && !isspace(*eptr) && *eptr != ',');
//  return val;
//}
//
//template<class T>
//istream& SMultiValueInput<T>::readValues(std::istream &in)
//{
//  values.clear();
//  string str;
//  while (!in.eof())
//  {
//    string tmp; in >> tmp; str += " " + tmp;
//  }
//  if (!str.empty())
//  {
//    const TChar *pStr = str.c_str();
//    for (; isspace(*pStr); pStr++);
//    
//    while (*pStr != 0)
//    {
//      Bool bSuccess = true;
//      T val = readValue(pStr, bSuccess);
//      if (!bSuccess)
//      {
//        insetstate(ios::failbit);
//        break;
//      }
//
//      if (maxNumValueslncl != 0 && values.size() >= maxNumValueslncl)
//      {
//        in.setstate(ios::failbit);
//        break;
//      }
//      values.push_back(val);
//      for (; isspace(*pStr); pStr++)
//        if (*pStr == ',')
//        {
//          pStr++;
//        }
//    }
//  }
//}

Bool TAppDecCfg::parseCfg( Int argc, TChar* argv[] )
{
  Bool do_help = false;
  string cfg_TargetDecLayerIdSetFile;
  string outputColourSpaceConvert;
  Int warnUnknowParameter = 0;
  string inputBitstreamsFileName;

  // FIXME: 
  po::Options opts;
  opts.addOptions()
  ("help",                      do_help,                    false,      "this help text")
  ("InpBitstreamSetFile,i",     inputBitstreamsFileName,    string(""), "input bitstream paths in this file")
	("OutBitstreamFile,o",				m_outBitstreamFileName,			string(""), "bitstream output file name")
  ("TargetWidth,-wdt",          m_iTargetWidth,             0,          "target picture width")
  ("TargetHeight,-hgt",         m_iTargetHeight,            0,          "target picture width")
  ("NumberOfTiles,-nt",         m_numberOfTiles,            0,          "this is for number of tiles")
  ("NumberOfTilesInColumn,-ntc",m_numberOfTilesInColumn,    0,          "this is for number of tiles in column")
  ("NUmberOfTilesInRow,-ntr",   m_numberOfTilesInRow,       0,          "this is for nubmer of tiles in row")
  ("TileUniformFlag",           m_tileUniformFlag,          true,        "this is a flag for tile uniform")
  ;

  po::setDefaults(opts);
  po::ErrorReporter err;
  const list<const TChar*>& argv_unhandled = po::scanArgv(opts, argc, (const TChar**) argv, err);

  for (list<const TChar*>::const_iterator it = argv_unhandled.begin(); it != argv_unhandled.end(); it++)
  {
    fprintf(stderr, "Unhandled argument ignored: `%s'\n", *it);
  }


  if (argc == 1 || do_help)
  {
    po::doHelp(cout, opts);
    return false;
  }

  if (err.is_errored)
  {
    if (!warnUnknowParameter)
    {
      /* errors have already been reported to stderr */
      return false;
    }
  }

  if (m_outBitstreamFileName.empty())
  {
    fprintf(stderr, "No output file specified, aborting\n");
    return false;
  }

  if (!inputBitstreamsFileName.empty())
  {
    ifstream inputBitstreamsFile(inputBitstreamsFileName.c_str(), ifstream::in);
    if (inputBitstreamsFile)
    {
      std::string lineFileName;
      while (std::getline(inputBitstreamsFile, lineFileName))
      {
        std::istringstream iStringStreamFileName(lineFileName);
        std::string fileName;
        if (!(iStringStreamFileName >> fileName))
        {
          break;
        }
        if (fileName.empty())
        {
          fprintf(stderr, "Input bitstreams file name is not good, aborting\n");
        }
        m_inBitstreamFileNames.push_back(std::string(fileName.c_str()));
       }
    }
    else
    {
      fprintf(stderr, "Input bitstreams file is not opend, aborting\n");
      return false;
    }
  }
  else
  {
    fprintf(stderr, "No input file specified, aborting\n");
    return false;
  }

  if (m_numberOfTiles == 0 || m_numberOfTilesInColumn == 0 || m_numberOfTilesInRow == 0 ||
    (m_numberOfTiles != m_numberOfTilesInColumn * m_numberOfTilesInRow)) 
  {
    fprintf(stderr, "Number of tiles is not collect, aborting\n");
    return false;
  }

  return true;
}

//! \}
