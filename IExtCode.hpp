#ifndef I_EXT_CODE_HPP
#define I_EXT_CODE_HPP

#include <matrix.h>
#include <mex.h>
#undef printf

#if defined TIME_DEL_NET_SIM_AS_SUB
	#define HEADER_PATHS_TDNS ..
#elif !defined HEADER_PATHS_TDNS
	#define HEADER_PATHS_TDNS .
#endif

#define SETQUOTE(A) #A

#define SETQUOTE_EXPAND(A) SETQUOTE(A)

#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/MexMemoryInterfacing/Headers/MexMem.hpp)
#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/RandomNumGen/Headers/FiltRandomTBB.hpp)

struct InternalVars;
struct InputArgs;

namespace IExtInterface
{
	// Forward declaration of all struct types
	struct SingleStateStruct;
	struct InputVarsStruct;
	struct StateOutStruct;
	struct OutputVarsStruct;
	struct InternalVarsStruct;

	struct OutOps {
		enum {
			I_EXT_REQ           = (1 << 0),
			I_EXT_GEN_STATE_REQ = (1 << 1),
			I_EXT_NEURON_REQ    = (1 << 2),
			I_RAND_NEURON_REQ   = (1 << 3),
		};
	};

	struct SingleStateStruct {
		MexVector<float>    Iext;
		MexVector<uint32_t> IExtGenState;
		int                 IExtNeuron;
		MexVector<int>      IRandNeuron;

		SingleStateStruct() :
			Iext(),
			IExtGenState(),
			IRandNeuron() {}

		void initialize(
			const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
			const InternalVars                      & SimulationInternalVars);
	};

	struct InputVarsStruct {
		// These are currently the Input Variables
		// State Variables area part of SingleStateStruct

		// Optional Simulation Algorithm Parameters
		float IRandAmplitude;        // Amplitude of Random poisson current stimulation
		float IRandDecayFactor;      // Random Current decay factor (Not used here)
		float IExtAmplitude;         // Amplitude of External current stumulation
		float AvgRandSpikeFreq;      // Average Random Frequency in Hertz

		uint32_t MajorTimePeriod;    // Major Time Period of External Stimulation
		uint32_t MajorOnTime;        // The Time amount during the period for which the external stimulation is applied
		uint32_t MinorTimePeriod;    // The Time Period of repetition of External Pattern during ON Time
		uint32_t NoOfNeurons;        // The No of neurons to which external stimulation is applied in each Minor Time Period

		size_t OutputControl;

		InputVarsStruct() {}
	};

	struct StateOutStruct {
		MexMatrix<float>    IextOut;
		MexMatrix<uint32_t> IExtGenStateOut;
		MexVector<int>      IExtNeuronOut;
		MexMatrix<int>      IRandNeuronOut;

		StateOutStruct() :
			IextOut(),
			IExtGenStateOut(),
			IExtNeuronOut(),
			IRandNeuronOut() {}

		void initialize(
			const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
			const InternalVars                      & SimulationInternalVars);
	};

	struct OutputVarsStruct {
		
		OutputVarsStruct() {}

		void initialize(
			const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
			const InternalVars                      & SimulationInternalVars);
	};

	struct InternalVarsStruct {

		float IRandAmplitude;
		float IRandDecayFactor;
		float IExtAmplitude;
		float AvgRandSpikeFreq;

		uint32_t MajorTimePeriod;
		uint32_t MajorOnTime;
		uint32_t MinorTimePeriod;
		uint32_t NoOfNeurons;

		size_t OutputControl;

		MexVector<float> Iext;
		XorShiftPlus IExtGen;
		size_t IExtNeuron;
		MexVector<int>IRandNeuron;

		InternalVarsStruct() :
			Iext(),
			IExtGen(), 
			IRandNeuron(),
			IExtNeuron(0) {}
	};

	size_t getOutputControl(char *OutputControlString);

	// Input and Initialization Functions
	void takeInputVarsFromMatlabStruct(
		IExtInterface::InputVarsStruct &IExtInputVarsStruct,
		const mxArray* MatlabInputStruct,
		InputArgs &SimulationInputArgs);

	void takeInitialStateFromMatlabStruct(
		IExtInterface::SingleStateStruct &IExtInitialStateStruct,
		const mxArray* MatlabInputStruct,
		InputArgs &SimulationInputArgs);

	void initInternalVariables(
		IExtInterface::InternalVarsStruct &IExtInternalVarsStruct,
		IExtInterface::InputVarsStruct    &IExtInputVarsStruct,
		IExtInterface::SingleStateStruct  &IExtInitialStateStruct,
		InputArgs &SimulationInputArgs);

	// C++ Output Functions
	void doOutput(
		IExtInterface::StateOutStruct     &IExtStateOutStruct,
		IExtInterface::OutputVarsStruct   &IExtOutputVarsStruct,
		IExtInterface::InternalVarsStruct &IExtInternalVarsStruct,
		InternalVars                      &SimulationInternalVars);

	void doSingleStateOutput(
		IExtInterface::SingleStateStruct  &IExtSingleStateStruct,
		IExtInterface::InternalVarsStruct &IExtInternalVarsStruct,
		InternalVars                      &SimulationInternalVars);

	void doInputVarsOutput(
		IExtInterface::InputVarsStruct    &IExtInInputVarsStruct,
		IExtInterface::InternalVarsStruct &IExtInternalVarsStruct,
		InternalVars                      &SimulationInternalVars);

	// Functions performing output to MATLAB Array
	mxArrayPtr putSingleStatetoMATLABStruct (IExtInterface::SingleStateStruct & IExtSingleStateStruct);
	mxArrayPtr putInputVarstoMATLABStruct   (IExtInterface::InputVarsStruct   & IExtInputVarsStruct);
	mxArrayPtr putStateVarstoMATLABStruct   (IExtInterface::StateOutStruct    & IExtStateOutStruct);
	mxArrayPtr putOutputVarstoMATLABStruct  (IExtInterface::OutputVarsStruct  & IExtOutputVarsStruct);

	// IExt Calculating and updating function
	void updateIExt(
		IExtInterface::InternalVarsStruct &IExtInternalVarsStruct,
		InternalVars                      &SimulationInternalVars);
};

#endif
