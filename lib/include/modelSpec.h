/*--------------------------------------------------------------------------
   Author: Thomas Nowotny
  
   Institute: Center for Computational Neuroscience and Robotics
              University of Sussex
              Falmer, Brighton BN1 9QJ, UK
  
   email to:  T.Nowotny@sussex.ac.uk
  
   initial version: 2010-02-07
   
   This file contains neuron model declarations.
  
--------------------------------------------------------------------------*/

//--------------------------------------------------------------------------
/*! \file modelSpec.h

\brief Header file that contains the class (struct) definition of neuronModel for 
defining a neuron model and the class definition of NNmodel for defining a neuronal network model. 
Part of the code generation and generated code sections.
*/
//--------------------------------------------------------------------------
#pragma once

#include "neuronGroup.h"
#include "synapseGroup.h"
#include "currentSource.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#ifdef MPI_ENABLE
#include <mpi.h>
#endif

#define NO_DELAY 0 //!< Macro used to indicate no synapse delay for the group (only one queue slot will be generated)

//!< Floating point precision to use for models
enum FloatType
{
    GENN_FLOAT,
    GENN_DOUBLE,
    GENN_LONG_DOUBLE,
};

//!< Precision to use for variables which store time
enum class TimePrecision
{
    DEFAULT,    //!< Time uses default model precision
    FLOAT,      //!< Time uses single precision - not suitable for long simulations
    DOUBLE,     //!< Time uses double precision - may reduce performance
};

// Wrappers to save typing when declaring VarInitialisers structures
template<typename S>
inline NewModels::VarInit initVar(const typename S::ParamValues &params)
{
    return NewModels::VarInit(S::getInstance(), params.getValues());
}

template<typename S>
inline typename std::enable_if<std::is_same<typename S::ParamValues, Snippet::ValueBase<0>>::value, NewModels::VarInit>::type initVar()
{
   return NewModels::VarInit(S::getInstance(), {});
}

inline NewModels::VarInit uninitialisedVar()
{
    return NewModels::VarInit(InitVarSnippet::Uninitialised::getInstance(), {});
}

template<typename S>
inline InitSparseConnectivitySnippet::Init initConnectivity(const typename S::ParamValues &params)
{
    return InitSparseConnectivitySnippet::Init(S::getInstance(), params.getValues());
}

template<typename S>
inline typename std::enable_if<std::is_same<typename S::ParamValues, Snippet::ValueBase<0>>::value, InitSparseConnectivitySnippet::Init>::type initConnectivity()
{
    return InitSparseConnectivitySnippet::Init(S::getInstance(), {});
}

inline InitSparseConnectivitySnippet::Init uninitialisedConnectivity()
{
    return InitSparseConnectivitySnippet::Init(InitSparseConnectivitySnippet::Uninitialised::getInstance(), {});
}

/*===============================================================
//! \brief class NNmodel for specifying a neuronal network model.
//
================================================================*/

class NNmodel
{
public:
    // Typedefines
    //=======================
    typedef std::map<std::string, NeuronGroup>::value_type NeuronGroupValueType;
    typedef std::map<std::string, SynapseGroup>::value_type SynapseGroupValueType;
    typedef std::map<std::string, std::pair<unsigned int, unsigned int>>::value_type SynapseGroupSubsetValueType;

    NNmodel();
    ~NNmodel();

    // PUBLIC MODEL FUNCTIONS
    //=======================
    void setName(const std::string&); //!< Method to set the neuronal network model name

    void setPrecision(FloatType); //!< Set numerical precision for floating point
    void setTimePrecision(TimePrecision timePrecision); //!< Set numerical precision for time
    void setDT(double); //!< Set the integration step size of the model
    void setTiming(bool); //!< Set whether timers and timing commands are to be included
    void setSeed(unsigned int); //!< Set the random seed (disables automatic seeding if argument not 0).

    //!< What is the default location for model state variables? 
    /*! Historically, everything was allocated on both the host AND device */
    void setDefaultVarLocation(VarLocation loc){ m_DefaultVarLocation = loc; } 

    //! What is the default location for sparse synaptic connectivity? 
    /*! Historically, everything was allocated on both the host AND device */
    void setDefaultSparseConnectivityLocation(VarLocation loc){ m_DefaultSparseConnectivityLocation = loc; }

    //!< Should compatible postsynaptic models and dendritic delay buffers be merged? 
    /*! This can significantly reduce the cost of updating neuron population but means that per-synapse group inSyn arrays can not be retrieved */
    void setMergePostsynapticModels(bool merge){ m_ShouldMergePostsynapticModels = merge; }

    //! Get the string literal that should be used to represent a value in the model's floating-point type
    std::string scalarExpr(const double) const;

    //! Are any variables in any populations in this model using zero-copy memory?
    bool zeroCopyInUse() const;

    //! Return number of synapse groups which require a presynaptic reset kernel to be run
    size_t getNumPreSynapseResetRequiredGroups() const;

    //! Is there reset logic to be run before the synapse kernel i.e. for dendritic delays
    bool isPreSynapseResetRequired() const{ return getNumPreSynapseResetRequiredGroups() > 0; }

    //! Gets the name of the neuronal network model
    const std::string &getName() const{ return name; }

    //! Gets the floating point numerical precision
    const std::string &getPrecision() const{ return ftype; }

    //! Gets the floating point numerical precision used to represent time
    std::string getTimePrecision() const;

    //! Gets the model integration step size
    double getDT() const { return dt; }

    //! Get the random seed
    unsigned int getSeed() const { return seed; }

    //! Are timers and timing commands enabled
    bool isTimingEnabled() const{ return timing; }

    //! Generate path for generated code
    std::string getGeneratedCodePath(const std::string &path, const std::string &filename) const;

    //! Finalise model
    // **YUCK** this really shouldn't be public
    void finalize();

    // PUBLIC INITIALISATION FUNCTIONS
    //================================
    const std::map<std::string, std::string> &getInitKernelParameters() const{ return m_InitKernelParameters; }

    //! Does this model require device initialisation kernel
    /*! **NOTE** this is for neuron groups and densely connected synapse groups only */
   // bool isDeviceInitRequired(int localHostID) const;

    //! Does this model require a device sparse initialisation kernel
    /*! **NOTE** this is for sparsely connected synapse groups only */
    //bool isDeviceSparseInitRequired() const;

    // PUBLIC NEURON FUNCTIONS
    //========================
    //! Get std::map containing local named NeuronGroup objects in model
    const std::map<std::string, NeuronGroup> &getLocalNeuronGroups() const{ return m_LocalNeuronGroups; }

    //! Get std::map containing remote named NeuronGroup objects in model
    const std::map<std::string, NeuronGroup> &getRemoteNeuronGroups() const{ return m_RemoteNeuronGroups; }

    //! Gets std::map containing names and types of each parameter that should be passed through to the neuron kernel
    const std::map<std::string, std::string> &getNeuronKernelParameters() const{ return neuronKernelParameters; }

    //! How many neurons are simulated locally in this model
    unsigned int getNumLocalNeurons() const;

    //! How many neurons are simulated remotely in this model
    unsigned int getNumRemoteNeurons() const;

    //! How many neurons make up the entire model
    unsigned int getNumNeurons() const{ return getNumLocalNeurons() + getNumRemoteNeurons(); }

    //! Find a neuron group by name
    const NeuronGroup *findNeuronGroup(const std::string &name) const;

    //! Find a neuron group by name
    NeuronGroup *findNeuronGroup(const std::string &name);

    //! Adds a new neuron group to the model using a neuron model managed by the user
    /*! \tparam NeuronModel type of neuron model (derived from NeuronModels::Base).
        \param name string containing unique name of neuron population.
        \param size integer specifying how many neurons are in the population.
        \param model neuron model to use for neuron group.
        \param paramValues parameters for model wrapped in NeuronModel::ParamValues object.
        \param varInitialisers state variable initialiser snippets and parameters wrapped in NeuronModel::VarValues object.
        \return pointer to newly created NeuronGroup */
    template<typename NeuronModel>
    NeuronGroup *addNeuronPopulation(const std::string &name, unsigned int size, const NeuronModel *model,
                                     const typename NeuronModel::ParamValues &paramValues,
                                     const typename NeuronModel::VarValues &varInitialisers,
                                     int hostID = 0, int deviceID = 0)
    {
#ifdef MPI_ENABLE
        // Determine the host ID
        int mpiHostID = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiHostID);

        // Pick map to add group to appropriately
        auto &groupMap = (hostID == mpiHostID) ? m_LocalNeuronGroups : m_RemoteNeuronGroups;
#else
        // If MPI is disabled always add to local neuron groups and zero host id
        auto &groupMap = m_LocalNeuronGroups;
        hostID = 0;
#endif

        // Add neuron group to map
        auto result = groupMap.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(name, size, model,
                                  paramValues.getValues(), varInitialisers.getInitialisers(), 
                                  m_DefaultVarLocation, hostID, deviceID));

        if(!result.second)
        {
            throw std::runtime_error("Cannot add a neuron population with duplicate name:" + name);
        }
        else
        {
            return &result.first->second;
        }
    }

    //! Adds a new neuron group to the model using a singleton neuron model created using standard DECLARE_MODEL and IMPLEMENT_MODEL macros
    /*! \tparam NeuronModel type of neuron model (derived from NeuronModels::Base).
        \param name string containing unique name of neuron population.
        \param size integer specifying how many neurons are in the population.
        \param paramValues parameters for model wrapped in NeuronModel::ParamValues object.
        \param varInitialisers state variable initialiser snippets and parameters wrapped in NeuronModel::VarValues object.
        \return pointer to newly created NeuronGroup */
    template<typename NeuronModel>
    NeuronGroup *addNeuronPopulation(const std::string &name, unsigned int size,
                                     const typename NeuronModel::ParamValues &paramValues, const typename NeuronModel::VarValues &varInitialisers,
                                     int hostID = 0, int deviceID = 0)
    {
        return addNeuronPopulation<NeuronModel>(name, size, NeuronModel::getInstance(), paramValues, varInitialisers, hostID, deviceID);
    }

    // PUBLIC SYNAPSE FUNCTIONS
    //=========================
    //! Get std::map containing local named SynapseGroup objects in model
    const std::map<std::string, SynapseGroup> &getLocalSynapseGroups() const{ return m_LocalSynapseGroups; }

    //! Get std::map containing remote named SynapseGroup objects in model
    const std::map<std::string, SynapseGroup> &getRemoteSynapseGroups() const{ return m_RemoteSynapseGroups; }

    //! Gets std::map containing names and types of each parameter that should be passed through to the synapse kernel
    const std::map<std::string, std::string> &getSynapseKernelParameters() const{ return synapseKernelParameters; }

    //! Gets std::map containing names and types of each parameter that should be passed through to the postsynaptic learning kernel
    const std::map<std::string, std::string> &getSimLearnPostKernelParameters() const{ return simLearnPostKernelParameters; }

    //! Gets std::map containing names and types of each parameter that should be passed through to the synapse dynamics kernel
    const std::map<std::string, std::string> &getSynapseDynamicsKernelParameters() const{ return synapseDynamicsKernelParameters; }

    //! Find a synapse group by name
    const SynapseGroup *findSynapseGroup(const std::string &name) const;

    //! Find a synapse group by name
    SynapseGroup *findSynapseGroup(const std::string &name);    

    //! Adds a synapse population to the model using weight update and postsynaptic models managed by the user
    /*! \tparam WeightUpdateModel type of weight update model (derived from WeightUpdateModels::Base).
        \tparam PostsynapticModel type of postsynaptic model (derived from PostsynapticModels::Base).
        \param name string containing unique name of neuron population.
        \param mtype how the synaptic matrix associated with this synapse population should be represented.
        \param delaySteps integer specifying number of timesteps delay this synaptic connection should incur (or NO_DELAY for none)
        \param src string specifying name of presynaptic (source) population
        \param trg string specifying name of postsynaptic (target) population
        \param wum weight update model to use for synapse group.
        \param weightParamValues parameters for weight update model wrapped in WeightUpdateModel::ParamValues object.
        \param weightVarInitialisers weight update model state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param weightPreVarInitialisers weight update model presynaptic state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param weightPostVarInitialisers weight update model postsynaptic state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param psm postsynaptic model to use for synapse group.
        \param postsynapticParamValues parameters for postsynaptic model wrapped in PostsynapticModel::ParamValues object.
        \param postsynapticVarInitialisers postsynaptic model state variable initialiser snippets and parameters wrapped in NeuronModel::VarValues object.
        \return pointer to newly created SynapseGroup */
    template<typename WeightUpdateModel, typename PostsynapticModel>
    SynapseGroup *addSynapsePopulation(const std::string &name, SynapseMatrixType mtype, unsigned int delaySteps, const std::string& src, const std::string& trg,
                                       const WeightUpdateModel *wum, const typename WeightUpdateModel::ParamValues &weightParamValues, const typename WeightUpdateModel::VarValues &weightVarInitialisers, const typename WeightUpdateModel::PreVarValues &weightPreVarInitialisers, const typename WeightUpdateModel::PostVarValues &weightPostVarInitialisers,
                                       const PostsynapticModel *psm, const typename PostsynapticModel::ParamValues &postsynapticParamValues, const typename PostsynapticModel::VarValues &postsynapticVarInitialisers,
                                       const InitSparseConnectivitySnippet::Init &connectivityInitialiser = uninitialisedConnectivity())
    {
        // Get source and target neuron groups
        auto srcNeuronGrp = findNeuronGroup(src);
        auto trgNeuronGrp = findNeuronGroup(trg);

#ifdef MPI_ENABLE
        // Get host ID of target neuron group
        const int hostID = trgNeuronGrp->getClusterHostID();

        // Determine the host ID
        int mpiHostID = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiHostID);

        // Pick map to add group to appropriately
        auto &groupMap = (hostID == mpiHostID) ? m_LocalSynapseGroups : m_RemoteSynapseGroups;
#else
        // If MPI is disabled always add to local synapse groups
        auto &groupMap = m_LocalSynapseGroups;
#endif

        // Add synapse group to map
        auto result = groupMap.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(name, mtype, delaySteps,
                                  wum, weightParamValues.getValues(), weightVarInitialisers.getInitialisers(), weightPreVarInitialisers.getInitialisers(), weightPostVarInitialisers.getInitialisers(),
                                  psm, postsynapticParamValues.getValues(), postsynapticVarInitialisers.getInitialisers(),
                                  srcNeuronGrp, trgNeuronGrp,
                                  connectivityInitialiser, m_DefaultVarLocation, m_DefaultSparseConnectivityLocation));

        if(!result.second)
        {
            throw std::runtime_error("Cannot add a synapse population with duplicate name:" + name);
        }
        else
        {
            return &result.first->second;
        }
    }

    //! Adds a synapse population to the model using singleton weight update and postsynaptic models created using standard DECLARE_MODEL and IMPLEMENT_MODEL macros
    /*! \tparam WeightUpdateModel type of weight update model (derived from WeightUpdateModels::Base).
        \tparam PostsynapticModel type of postsynaptic model (derived from PostsynapticModels::Base).
        \param name string containing unique name of neuron population.
        \param mtype how the synaptic matrix associated with this synapse population should be represented.
        \param delaySteps integer specifying number of timesteps delay this synaptic connection should incur (or NO_DELAY for none)
        \param src string specifying name of presynaptic (source) population
        \param trg string specifying name of postsynaptic (target) population
        \param weightParamValues parameters for weight update model wrapped in WeightUpdateModel::ParamValues object.
        \param weightVarInitialisers weight update model state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param postsynapticParamValues parameters for postsynaptic model wrapped in PostsynapticModel::ParamValues object.
        \param postsynapticVarInitialisers postsynaptic model state variable initialiser snippets and parameters wrapped in NeuronModel::VarValues object.
        \return pointer to newly created SynapseGroup */
    template<typename WeightUpdateModel, typename PostsynapticModel>
    SynapseGroup *addSynapsePopulation(const std::string &name, SynapseMatrixType mtype, unsigned int delaySteps, const std::string& src, const std::string& trg,
                                       const typename WeightUpdateModel::ParamValues &weightParamValues, const typename WeightUpdateModel::VarValues &weightVarInitialisers,
                                       const typename PostsynapticModel::ParamValues &postsynapticParamValues, const typename PostsynapticModel::VarValues &postsynapticVarInitialisers,
                                       const InitSparseConnectivitySnippet::Init &connectivityInitialiser = uninitialisedConnectivity())
    {
        // Create empty pre and postsynaptic weight update variable initialisers
        typename WeightUpdateModel::PreVarValues weightPreVarInitialisers;
        typename WeightUpdateModel::PostVarValues weightPostVarInitialisers;

        return addSynapsePopulation(name, mtype, delaySteps, src, trg,
                                    WeightUpdateModel::getInstance(), weightParamValues, weightVarInitialisers, weightPreVarInitialisers, weightPostVarInitialisers,
                                    PostsynapticModel::getInstance(), postsynapticParamValues, postsynapticVarInitialisers,
                                    connectivityInitialiser);
    }

    //! Adds a synapse population to the model using singleton weight update and postsynaptic models created using standard DECLARE_MODEL and IMPLEMENT_MODEL macros
    /*! \tparam WeightUpdateModel type of weight update model (derived from WeightUpdateModels::Base).
        \tparam PostsynapticModel type of postsynaptic model (derived from PostsynapticModels::Base).
        \param name string containing unique name of neuron population.
        \param mtype how the synaptic matrix associated with this synapse population should be represented.
        \param delaySteps integer specifying number of timesteps delay this synaptic connection should incur (or NO_DELAY for none)
        \param src string specifying name of presynaptic (source) population
        \param trg string specifying name of postsynaptic (target) population
        \param weightParamValues parameters for weight update model wrapped in WeightUpdateModel::ParamValues object.
        \param weightVarInitialisers weight update model per-synapse state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param weightPreVarInitialisers weight update model presynaptic state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param weightPostVarInitialisers weight update model postsynaptic state variable initialiser snippets and parameters wrapped in WeightUpdateModel::VarValues object.
        \param postsynapticParamValues parameters for postsynaptic model wrapped in PostsynapticModel::ParamValues object.
        \param postsynapticVarInitialisers postsynaptic model state variable initialiser snippets and parameters wrapped in NeuronModel::VarValues object.
        \return pointer to newly created SynapseGroup */
    template<typename WeightUpdateModel, typename PostsynapticModel>
    SynapseGroup *addSynapsePopulation(const std::string &name, SynapseMatrixType mtype, unsigned int delaySteps, const std::string& src, const std::string& trg,
                                       const typename WeightUpdateModel::ParamValues &weightParamValues, const typename WeightUpdateModel::VarValues &weightVarInitialisers, const typename WeightUpdateModel::PreVarValues &weightPreVarInitialisers, const typename WeightUpdateModel::PostVarValues &weightPostVarInitialisers,
                                       const typename PostsynapticModel::ParamValues &postsynapticParamValues, const typename PostsynapticModel::VarValues &postsynapticVarInitialisers,
                                       const InitSparseConnectivitySnippet::Init &connectivityInitialiser = uninitialisedConnectivity())
    {
        return addSynapsePopulation(name, mtype, delaySteps, src, trg,
                                    WeightUpdateModel::getInstance(), weightParamValues, weightVarInitialisers, weightPreVarInitialisers, weightPostVarInitialisers,
                                    PostsynapticModel::getInstance(), postsynapticParamValues, postsynapticVarInitialisers,
                                    connectivityInitialiser);

    }

    // PUBLIC CURRENT SOURCE FUNCTIONS
    //================================

    //! Get std::map containing local named CurrentSource objects in model
    const std::map<std::string, CurrentSource> &getLocalCurrentSources() const{ return m_LocalCurrentSources; }

    //! Get std::map containing remote named CurrentSource objects in model
    const std::map<std::string, CurrentSource> &getRemoteCurrentSources() const{ return m_RemoteCurrentSources; }

    //! Gets std::map containing names and types of each parameter that should be passed through to the current source kernel
    const std::map<std::string, std::string> &getCurrentSourceKernelParameters() const{ return currentSourceKernelParameters; }

    //! Find a current source by name
    const CurrentSource *findCurrentSource(const std::string &name) const;

    //! Find a current source by name
    CurrentSource *findCurrentSource(const std::string &name);

    //! Adds a new current source to the model using a current source model managed by the user
    /*! \tparam CurrentSourceModel type of current source model (derived from CurrentSourceModels::Base).
        \param name string containing unique name of current source.
        \param model current source model to use for current source.
        \param targetNeuronGroupName string name of the target neuron group
        \param paramValues parameters for model wrapped in CurrentSourceModel::ParamValues object.
        \param varInitialisers state variable initialiser snippets and parameters wrapped in CurrentSource::VarValues object.
        \return pointer to newly created CurrentSource */
    template<typename CurrentSourceModel>
    CurrentSource *addCurrentSource(const std::string &currentSourceName, const CurrentSourceModel *model,
                                    const std::string &targetNeuronGroupName,
                                    const typename CurrentSourceModel::ParamValues &paramValues,
                                    const typename CurrentSourceModel::VarValues &varInitialisers)
    {
        auto targetGroup = findNeuronGroup(targetNeuronGroupName);

#ifdef MPI_ENABLE
        // Get host ID of target neuron group
        const int hostID = targetGroup->getClusterHostID();

        // Determine the host ID
        int mpiHostID = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &mpiHostID);

        // Pick map to add group to appropriately
        auto &groupMap = (hostID == mpiHostID) ? m_LocalCurrentSources : m_RemoteCurrentSources;
#else
        // If MPI is disabled always add to local current sources
        auto &groupMap = m_LocalCurrentSources;
#endif

        // Add current source to map
        auto result = groupMap.emplace(std::piecewise_construct,
            std::forward_as_tuple(currentSourceName),
            std::forward_as_tuple(currentSourceName, model,
                                  paramValues.getValues(), varInitialisers.getInitialisers()));

        if(!result.second)
        {
            throw std::runtime_error("Cannot add a current source with duplicate name:" + currentSourceName);
        }
        else
        {
            targetGroup->injectCurrent(&result.first->second);
            return &result.first->second;
        }
    }

    //! Adds a new current source to the model using a singleton current source model created using standard DECLARE_MODEL and IMPLEMENT_MODEL macros
    /*! \tparam CurrentSourceModel type of neuron model (derived from CurrentSourceModel::Base).
        \param currentSourceName string containing unique name of current source.
        \param targetNeuronGroupName string name of the target neuron group
        \param paramValues parameters for model wrapped in CurrentSourceModel::ParamValues object.
        \param varInitialisers state variable initialiser snippets and parameters wrapped in CurrentSourceModel::VarValues object.
        \return pointer to newly created CurrentSource */
    template<typename CurrentSourceModel>
    CurrentSource *addCurrentSource(const std::string &currentSourceName, const std::string &targetNeuronGroupName,
                                    const typename CurrentSourceModel::ParamValues &paramValues,
                                    const typename CurrentSourceModel::VarValues &varInitialisers)
    {
        return addCurrentSource<CurrentSourceModel>(currentSourceName, CurrentSourceModel::getInstance(),
                                targetNeuronGroupName, paramValues, varInitialisers);
    }


private:
    //--------------------------------------------------------------------------
    // Private members
    //--------------------------------------------------------------------------
    //!< Named local neuron groups
    std::map<std::string, NeuronGroup> m_LocalNeuronGroups;

    //!< Named remote neuron groups
    std::map<std::string, NeuronGroup> m_RemoteNeuronGroups;

    //!< Named local synapse groups
    std::map<std::string, SynapseGroup> m_LocalSynapseGroups;

    //!< Named remote synapse groups
    std::map<std::string, SynapseGroup> m_RemoteSynapseGroups;

    //!< Named local current sources
    std::map<std::string, CurrentSource> m_LocalCurrentSources;

    //!< Named remote current sources
    std::map<std::string, CurrentSource> m_RemoteCurrentSources;

    // Kernel members
    std::map<std::string, std::string> m_InitKernelParameters;
    std::map<std::string, std::string> neuronKernelParameters;
    std::map<std::string, std::string> synapseKernelParameters;
    std::map<std::string, std::string> simLearnPostKernelParameters;
    std::map<std::string, std::string> synapseDynamicsKernelParameters;
    std::map<std::string, std::string> currentSourceKernelParameters;

     // Model members
    std::string name;               //!< Name of the neuronal newtwork model
    std::string ftype;              //!< Type of floating point variables (float, double, ...; default: float)
    TimePrecision m_TimePrecision;  //!< Type of floating point variables used to store time
    double dt;                      //!< The integration time step of the model
    bool timing;
    unsigned int seed;

    //! Should compatible postsynaptic models and dendritic delay buffers be merged? 
    //! This can significantly reduce the cost of updating neuron population but means that per-synapse group inSyn arrays can not be retrieved
    bool m_MergePostsynapticModels; 

    //!< What is the default location for model state variables? Historically, everything was allocated on both host AND device
    VarLocation m_DefaultVarLocation;  

    //! What is the default location for sparse synaptic connectivity? Historically, everything was allocated on both the host AND device
    VarLocation m_DefaultSparseConnectivityLocation; 

    //!< Should compatible postsynaptic models and dendritic delay buffers be merged? 
    /*! This can significantly reduce the cost of updating neuron population but means that per-synapse group inSyn arrays can not be retrieved */
    bool m_ShouldMergePostsynapticModels; 
};
