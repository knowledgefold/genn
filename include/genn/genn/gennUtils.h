#pragma once

// Standard C++ includes
#include <string>
#include <vector>

// GeNN includes
#include "gennExport.h"
#include "models.h"
#include "variableImplementation.h"

//--------------------------------------------------------------------------
// Utils
//--------------------------------------------------------------------------
namespace Utils
{
//--------------------------------------------------------------------------
//! \brief Does the code string contain any functions requiring random number generator
//--------------------------------------------------------------------------
GENN_EXPORT bool isRNGRequired(const std::string &code);

//--------------------------------------------------------------------------
//! \brief Does the model with the vectors of variable initialisers and modes require an RNG for the specified init location i.e. host or device
//--------------------------------------------------------------------------
GENN_EXPORT bool isInitRNGRequired(const std::vector<Models::VarInit> &varInitialisers);

//! Automatically determine the default implementation for this vector of variables
GENN_EXPORT void autoDetermineImplementation(const std::vector<Models::VarInit> &initialisers, const Models::Base::VarVec &vars,
                                             std::vector<VarImplementation> &implementations);

//--------------------------------------------------------------------------
//! \brief Function to determine whether a string containing a type is a pointer
//--------------------------------------------------------------------------
GENN_EXPORT bool isTypePointer(const std::string &type);

//--------------------------------------------------------------------------
//! \brief Assuming type is a string containing a pointer type, function to return the underlying type
//--------------------------------------------------------------------------
GENN_EXPORT std::string getUnderlyingType(const std::string &type);

}   // namespace Utils
