#pragma once

// Standard includes
#include <string>

// GeNN includes
#include "codeGenUtils.h"
#include "newNeuronModels.h"

// Forward declarations
class NeuronGroup;
class NNmodel;

//----------------------------------------------------------------------------
// NameIterCtx
//----------------------------------------------------------------------------
template<typename Container>
struct NameIterCtx
{
    typedef PairKeyConstIter<typename Container::const_iterator> NameIter;

    NameIterCtx(Container &&c) :
        container(std::move(c)), nameBegin(std::begin(c)), nameEnd(std::end(c)){}

    const Container container;
    const NameIter nameBegin;
    const NameIter nameEnd;
};

//----------------------------------------------------------------------------
// Typedefines
//----------------------------------------------------------------------------
typedef NameIterCtx<NewModels::Base::StringPairVec> VarNameIterCtx;
typedef NameIterCtx<NewModels::Base::DerivedParamVec> DerivedParamNameIterCtx;
typedef NameIterCtx<NewModels::Base::StringPairVec> ExtraGlobalParamNameIterCtx;

//----------------------------------------------------------------------------
// Standard substitution functins
//----------------------------------------------------------------------------
namespace StandardSubstitutions
{
void postSynapseCurrentConverter(
    std::string &psCode,          //!< the code string to work on
    const NNmodel &model,    //!< **TEMP**
    int synPopID,            //!< **TEMP**
    const std::string &ngName,
    const NeuronGroup &ng,
    const VarNameIterCtx &nmVars,
    const DerivedParamNameIterCtx &nmDerivedParams,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams);

void postSynapseDecay(
    std::string &pdCode,
    const NNmodel &model,    //!< **TEMP**
    int synPopID,            //!< **TEMP**
    const std::string &ngName,
    const NeuronGroup &ng,
    const VarNameIterCtx &nmVars,
    const DerivedParamNameIterCtx &nmDerivedParams,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams,
    const std::string &ftype);

void neuronThresholdCondition(
    std::string &thCode,
    const std::string &ngName,
    const NeuronGroup &ng,
    const VarNameIterCtx &nmVars,
    const DerivedParamNameIterCtx &nmDerivedParams,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams,
    const std::string &ftype);

void neuronSim(
    std::string &sCode,
    const std::string &ngName,
    const NeuronGroup &ng,
    const VarNameIterCtx &nmVars,
    const DerivedParamNameIterCtx &nmDerivedParams,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams,
    const std::string &ftype);

void neuronSpikeEventCondition(
    std::string &eCode,
    const std::string &ngName,
    const VarNameIterCtx &nmVars,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams,
    const std::string &ftype);

void neuronReset(
    std::string &rCode,
    const std::string &ngName,
    const NeuronGroup &ng,
    const VarNameIterCtx &nmVars,
    const DerivedParamNameIterCtx &nmDerivedParams,
    const ExtraGlobalParamNameIterCtx &nmExtraGlobalParams,
    const std::string &ftype);

}   // StandardSubstitions