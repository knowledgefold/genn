#pragma once


// GeNN includes
#include "modelSpec.h"

//------------------------------------------------------------------------
// ModelSpecInternal
//------------------------------------------------------------------------
class ModelSpecInternal : public ModelSpec
{
public:
    //------------------------------------------------------------------------
    // Public API
    //------------------------------------------------------------------------
    using ModelSpec::getNeuronGroups;
    using ModelSpec::getSynapseGroups;
    using ModelSpec::getLocalCurrentSources;
    using ModelSpec::getMergedNeuronGroups;
    using ModelSpec::getMergedSynapseGroups;
    using ModelSpec::getMergedNeuronInitGroups;
    using ModelSpec::getMergedSynapseInitGroups;
    using ModelSpec::getMergedSynapseConnectivityInitGroups;


    using ModelSpec::finalize;

    using ModelSpec::scalarExpr;

    using ModelSpec::zeroCopyInUse;
};
