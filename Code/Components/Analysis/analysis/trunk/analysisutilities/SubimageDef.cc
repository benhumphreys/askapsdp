/// @file
///
/// @brief Define and access subimages of a FITS file.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
/// 

#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <askapparallel/AskapParallel.h>

#include <analysisutilities/AnalysisUtilities.h>

#include <gsl/gsl_sf_gamma.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

#define WCSLIB_GETWCSTAB // define this so that we don't try and redefine 
                         //  wtbarr (this is a problem when using gcc v.4+)
#include <fitsio.h>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".analysisutilities");

namespace askap
{
  namespace analysis
  {

    SubimageDef::SubimageDef()
    {
      itsNAxis = 0; 
      itsNSubX = 1;
      itsNSubY = 1;
      itsNSubZ = 1;
    }


    SubimageDef& SubimageDef::operator= (const SubimageDef& s)
    {
      /// @details Copy constructor for SubimageDef, that does a deep
      /// copy of the itsNSub and itsOverlap arrays.

      if(this==&s) return *this;
      this->itsNSubX = s.itsNSubX;
      this->itsNSubY = s.itsNSubY;
      this->itsNSubZ = s.itsNSubZ;
      this->itsOverlapX = s.itsOverlapX;
      this->itsOverlapY = s.itsOverlapY;
      this->itsOverlapZ = s.itsOverlapZ;
      this->itsNAxis = s.itsNAxis;
      for(int i=0;i<this->itsNAxis;i++){
	this->itsNSub[i] = s.itsNSub[i];
	this->itsOverlap[i] = s.itsOverlap[i];
      }
      return *this;
    }


    void SubimageDef::define(const LOFAR::ACC::APS::ParameterSet& parset)
    {

      /// @details Define all the necessary variables within the
      /// SubimageDef class. The image (given by the parameter "image"
      /// in the parset) is to be split up according to the nsubx/y/z
      /// parameters, with overlaps in each direction given by the
      /// overlapx/y/z parameters (these are in pixels).
      ///
      /// The Duchamp function duchamp::FitsHeader::defineWCS() is
      /// used to extract the WCS parameters from the FITS
      /// header. These determine which axes are the x, y and z
      /// axes. The number of axes is also determined from the WCS
      /// parameter set.
      ///
      /// @param parset The parameter set holding info on how to divide the image.

      this->itsImageName = parset.getString("image");

      this->itsNSubX = parset.getInt16("nsubx",1);
      this->itsNSubY = parset.getInt16("nsuby",1);
      this->itsNSubZ = parset.getInt16("nsubz",1);
      this->itsOverlapX = parset.getInt16("overlapx",0);
      this->itsOverlapY = parset.getInt16("overlapy",0);
      this->itsOverlapZ = parset.getInt16("overlapz",0);  

      duchamp::Param tempPar; // This is needed for defineWCS(), but we don't care about the contents.
      duchamp::FitsHeader imageHeader;
      imageHeader.defineWCS(this->itsImageName,tempPar);
      this->itsNAxis = imageHeader.WCS().naxis;
      const int lng = imageHeader.WCS().lng;
      const int lat = imageHeader.WCS().lat;
      const int spec = imageHeader.WCS().spec;

      this->itsNSub = new int[this->itsNAxis];
      this->itsOverlap = new int[this->itsNAxis];
      for(int i=0;i<this->itsNAxis;i++){
	if(i==lng){ 
	  this->itsNSub[i]=this->itsNSubX; 
	  this->itsOverlap[i]=this->itsOverlapX; 
	}
	else if(i==lat){
	  this->itsNSub[i]=this->itsNSubY; 
	  this->itsOverlap[i]=this->itsOverlapY;
	}
	else if(i==spec){
	  this->itsNSub[i]=this->itsNSubZ;
	  this->itsOverlap[i]=this->itsOverlapZ; 
	}
	else{
	  this->itsNSub[i] = 1; 
	  this->itsOverlap[i] = 0;
	}
      }

    }


    duchamp::Section SubimageDef::section(int workerNum)
    {

      /// @details Return the subsection object for the given worker
      /// number. (These start at 0). The subimages are tiled across
      /// the cube with the x-direction varying quickest, then y, then
      /// z. The dimensions of the array are obtained with the
      /// getFITSdimensions(std::string) function.
      /// @return A duchamp::Section object containing all information
      /// on the subsection.

      long *dimAxes = getFITSdimensions(this->itsImageName);
      long start = 0;
      
      long sub[3];
      sub[0] = workerNum % this->itsNSub[0];
      sub[1] = (workerNum % (this->itsNSub[0]*this->itsNSub[1])) / this->itsNSub[0];
      sub[2] = workerNum / (this->itsNSub[0]*this->itsNSub[1]);

      std::stringstream section;

      for(int i=0;i<this->itsNAxis;i++){
	    
	if(this->itsNSub[i] > 1){
	  int min = std::max( start, sub[i]*(dimAxes[i]/this->itsNSub[i]) - this->itsOverlap[i]/2 ) + 1;
	  int max = std::min( dimAxes[i], (sub[i]+1)*(dimAxes[i]/this->itsNSub[i]) + this->itsOverlap[i]/2 );
	  section << min << ":" << max;
	}
	else section << "*";
	if(i != this->itsNAxis-1) section << ",";
	  
      }
      std::string secstring = "["+section.str()+"]";
      duchamp::Section sec(secstring);
      std::vector<long> dim(this->itsNAxis);
      for(int i=0;i<this->itsNAxis;i++) dim[i] = dimAxes[i];
      sec.parse(dim);

      return sec;
    }


  }

}