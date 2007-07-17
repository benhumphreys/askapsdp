/// @file
///
/// WProjectVisGridder: W projection gridding

/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_
#define CONRAD_SYNTHESIS_WPROJECTVISGRIDDER_H_

#include <gridding/SphFuncVisGridder.h>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Visibility gridder using W projection
    /// @ingroup gridding
    class WProjectVisGridder : public SphFuncVisGridder
    {
      public:

/// @brief Construct a gridder for W projection
/// @param wmax Maximum baseline (wavelengths)
/// @param nwplanes Number of w planes
/// @param cutoff Cutoff in determining support e.g. 10^-3 of the peak
/// @param overSample Oversampling (currently limited to <=1)
//
        WProjectVisGridder(const double wmax, const int nwplanes,
          const double cutoff, const int overSample);

        virtual ~WProjectVisGridder();

      protected:
      /// Offset into convolution function
      /// @param row Row number
      /// @param chan Channel number
        virtual int cOffset(int row, int chan);
        /// Initialize convolution function
        /// @param idi Data access iterator
        /// @param cellSize Cell size in wavelengths
        /// @param shape Shape of grid
        virtual void initConvolutionFunction(IDataSharedIter& idi, 
          const casa::Vector<double>& cellSize,
          const casa::IPosition& shape);
      private:
        /// Scaling
        double itsWScale;
        /// Number of w planes
        int itsNWPlanes;
        /// Threshold for cutoff of convolution function
        double itsCutoff;
        /// Mapping from row and channel to planes
        casa::Matrix<int> itsCMap;
    };
  }
}
#endif
