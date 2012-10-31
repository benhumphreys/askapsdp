/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#ifndef ASKAP_ANALYSIS_EXTRACTION_SOURCESPECTRUM_H_
#define ASKAP_ANALYSIS_EXTRACTION_SOURCESPECTRUM_H_
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <Common/ParameterSet.h>
#include <utils/PolConverter.h>
#include <casa/Arrays/Vector.h>

namespace askap {

  namespace analysis {
    
    /// @brief Class to handle the extraction of a spectrum for a given RadioSource. 

    /// @details This class is aimed primarily at solving the problem
    /// of extracting the integrated spectrum from a cube for a
    /// previously-detected object, usually a continuum source. One
    /// example would be extracting the spectra in all Stokes
    /// parameters of a continuum source detected in Stokes I (for
    /// instance, in an MFS image).
    ///
    /// The spectrum is extracted by summing over an NxN box, centred
    /// on the peak pixel of the RadioSource. The summed flux can be
    /// optionally scaled by the beam size to give the flux of an
    /// unresolved object.

   class SourceSpectrumExtractor : public SpectralBoxExtractor
    {
    public:
      SourceSpectrumExtractor(){};
      SourceSpectrumExtractor(const LOFAR::ParameterSet& parset);
      virtual ~SourceSpectrumExtractor(){};
      SourceSpectrumExtractor(const SourceSpectrumExtractor& other);
      SourceSpectrumExtractor& operator=(const SourceSpectrumExtractor& other);
      
      void setBoxWidth(int w){itsBoxWidth=w; setBeamScale();};
      bool doScale(){return itsFlagDoScale;};
      void setFlagDoScale(bool b){itsFlagDoScale=b; setBeamScale();};

      void setBeamScale();

      void extract();

    protected:
      bool itsFlagDoScale;
      float itsBeamScaleFactor;
      bool itsFlagUseDetection;

    };


  }

}


#endif
