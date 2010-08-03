/// @file
/// @brief Class for a BasisFunction-Clean-based deconvolver
/// @details This interface class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
///  
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///

#ifndef I_DECONVOLVERBASISFUNCTION_H
#define I_DECONVOLVERBASISFUNCTION_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverState.h>
#include <deconvolution/DeconvolverControl.h>
#include <deconvolution/DeconvolverMonitor.h>
#include <deconvolution/BasisFunction.h>

namespace askap {

  namespace synthesis {

    /// @brief Class for a deconvolver using the BasisFunction Clean algorithm
    /// @details This base class defines a deconvolver used to estimate an
    /// image from a dirty image, psf optionally using a mask and a weights image.
    /// The template argument T is the type, and FT is the transform
    /// e.g. DeconvolverBasisFunction<Double, DComplex>
    /// @ingroup Deconvolver
    template<class T, class FT>
    class DeconvolverBasisFunction : public DeconvolverBase<T, FT> {

    public:
      typedef boost::shared_ptr<DeconvolverBasisFunction<T, FT> > ShPtr;
  
      virtual ~DeconvolverBasisFunction();
  
      /// @brief Construct from dirty image and psf
      /// @detail Construct a deconvolver from a dirty image and
      /// the corresponding PSF
      /// @param[in] dirty Dirty image (array)
      /// @param[in] psf Point Spread Function (array)
      DeconvolverBasisFunction(Array<T>& dirty, Array<T>& psf);

      /// @brief Set the basis function to be used
      /// @details The algorithm can work with different basis functions
      /// PointBasisFunction, MultiScaleBasisFunction (without and with
      /// orthogonalisation.
      
      void setBasisFunction(boost::shared_ptr<BasisFunction<T> > bf);

      /// @brief Return the basis function to be used
      /// @details The algorithm can work with different basis functions
      /// PointBasisFunction, MultiScaleBasisFunction (without and with
      /// orthogonalisation.
      boost::shared_ptr<BasisFunction<T> > basisFunction();

      /// @brief Perform the deconvolution
      /// @detail This is the main deconvolution method.
      virtual bool deconvolve();

      /// @brief Initialize the deconvolution
      /// @detail Initialise e.g. set weighted mask
      virtual void initialise();

      /// @brief Initialize the deconvolution
      /// @detail Initialise e.g. set weighted mask
      virtual void finalise();

      /// @brief configure basic parameters of the solver
      /// @details This method encapsulates extraction of basic solver parameters from the parset.
      /// @param[in] parset parset
      virtual void configure(const LOFAR::ParameterSet &parset); 
      
    private:

      /// @brief Perform the deconvolution
      /// @detail This is the main deconvolution method.
      bool oneIteration();

      /// Residual images convolved with basis functions
      Array<T> itsResidualBasisFunction;

      /// Point spread functions convolved with basis functions
      Array<T> itsPSFBasisFunction;

      Bool itsUseCrossTerms;

      Matrix<T> itsCouplingMatrix;

      /// Point spread functions convolved with cross terms of basis functions
      Array<T> itsPSFCrossTerms;

      boost::shared_ptr<BasisFunction<T> > itsBasisFunction;
    };

  } // namespace synthesis

} // namespace askap

#include <deconvolution/DeconvolverBasisFunction.tcc>

#endif  // #ifndef I_DECONVOLVERBASISFUNCTION_H


