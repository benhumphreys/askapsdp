/// @file
/// @brief A method to set up converters and selectors from parset file
/// @details Parameters are currently passed around using parset files.
/// The methods declared in this file set up converters and selectors
/// from the ParameterSet object. This is probably a temporary solution.
/// This code can eventually become a part of some class (e.g. a DataSource
/// which returns selectors and converters with the defaults alread
/// applied according to the parset file).
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef ME_PARSET_INTERFACE_H
#define ME_PARSET_INTERFACE_H

// own includes
#include <fitting/Params.h>
#include <fitting/Solver.h>
#include <APS/ParameterSet.h>

namespace conrad {

namespace synthesis {

/// @brief set up images according to the parset file
/// @param[in] params Images to be created here
/// @param[in] parset a parset object to read the parameters from
/// @ingroup measurementequation
  void operator<<(conrad::scimath::Params::ShPtr& params, const LOFAR::ACC::APS::ParameterSet &parset);

/// @brief set up solver according to the parset file
/// @param[in] solver Pointer to solver to be created
/// @param[in] parset a parset object to read the parameters from
/// @ingroup measurementequation
  void operator<<(conrad::scimath::Solver::ShPtr& solver, const LOFAR::ACC::APS::ParameterSet &parset);

} // namespace synthesis

} // namespace conrad


#endif
