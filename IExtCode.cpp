#include "IExtCode.hpp"
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <algorithm>

#include "../Network.hpp"
#include "../NeuronSim.hpp"

#include <matrix.h>

#if defined TIME_DEL_NET_SIM_AS_SUB
	#define HEADER_PATHS_TDNS ..
#elif !defined HEADER_PATHS_TDNS
	#define HEADER_PATHS_TDNS .
#endif

#define SETQUOTE(A) #A

#define SETQUOTE_EXPAND(A) SETQUOTE(A)

#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/MexMemoryInterfacing/Headers/MexMem.hpp)
#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/MexMemoryInterfacing/Headers/GenericMexIO.hpp)
#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/RandomNumGen/Headers/FiltRandomTBB.hpp)

////////////////////////////////////////////////////////
// Input and Initialization functions 
////////////////////////////////////////////////////////
size_t IExtInterface::getOutputControl(char * OutputControlString)
{
	std::vector<std::string> OutputControlOptions;
	StringSplit(OutputControlString, " ,-", OutputControlOptions);
	
	// Defining case insensitive comparison function
	auto CaseInsensitiveCharComp = [](char c1, char c2) -> bool { return std::tolower(c1) == std::tolower(c2);};
	auto iEqual = [&](const std::string &s1, const std::string &s2) -> bool {
		return (s1.size() == s2.size()) && std::equal(s1.begin(), s1.end(), s2.begin(), CaseInsensitiveCharComp);
	};

	// Defining return variable
	size_t OutputControlWord = 0;

	for (auto OutContOpt : OutputControlOptions) {
		// Split the current OutputControlOption by '.'
		std::vector<std::string> OutputControlOptionParts;
		StringSplit(OutContOpt.data(), ".", OutputControlOptionParts);

		bool AddorRemove = true; // TRUE for Add

		if (iEqual(OutputControlOptionParts[0], "FSF")) {
			OutputControlWord |=
				IExtInterface::OutOps::I_EXT_GEN_STATE_REQ |
				IExtInterface::OutOps::I_EXT_REQ |
				IExtInterface::OutOps::I_EXT_NEURON_REQ |
				IExtInterface::OutOps::I_RAND_NEURON_REQ;
		}

		if (iEqual(OutputControlOptionParts[0], "IExt") || iEqual(OutputControlOptionParts[0], "/IExt")
			&& OutputControlOptionParts.size() == 2) {
			// Ascertain Add or Remove
			if (OutputControlOptionParts[0][0] == '/') {
				AddorRemove = false;
			}

			// Check out different Output Option cases
			if (iEqual(OutputControlOptionParts[1], "IExtGenState")) { 
				OutputControlWord = (AddorRemove) ?
					OutputControlWord |  IExtInterface::OutOps::I_EXT_GEN_STATE_REQ: 
					OutputControlWord & ~IExtInterface::OutOps::I_EXT_GEN_STATE_REQ;
			}
			if (iEqual(OutputControlOptionParts[1], "Iext")) {
				OutputControlWord = (AddorRemove) ?
					OutputControlWord |  IExtInterface::OutOps::I_EXT_REQ: 
					OutputControlWord & ~IExtInterface::OutOps::I_EXT_REQ;
			}
			if (iEqual(OutputControlOptionParts[1], "IExtNeuron")) {
				OutputControlWord = (AddorRemove) ?
					OutputControlWord |  IExtInterface::OutOps::I_EXT_NEURON_REQ:
					OutputControlWord & ~IExtInterface::OutOps::I_EXT_NEURON_REQ;
			}
			if (iEqual(OutputControlOptionParts[1], "IRandNeuron")) {
				OutputControlWord = (AddorRemove) ?
					OutputControlWord |  IExtInterface::OutOps::I_RAND_NEURON_REQ: 
					OutputControlWord & ~IExtInterface::OutOps::I_RAND_NEURON_REQ;
			}
		}
	}

	return OutputControlWord;
}

void IExtInterface::takeInputVarsFromMatlabStruct(
	IExtInterface::InputVarsStruct & IExtInputVarsStruct,
	const mxArray* IExtMatlabInputStruct, 
	InputArgs & SimulationInputArgs)
{
	// Aliasing input argument structs
	auto & InputVars   = IExtInputVarsStruct;
	auto & MatlabInput = IExtMatlabInputStruct;
	auto & SimIntVars  = SimulationInputArgs;

	// Giving Default Values to Optional Simulation Algorithm Parameters
	InputVars.IRandDecayFactor = 2.0f / 3;
	InputVars.IRandAmplitude = 20.0f;
	InputVars.IExtAmplitude  = 20.0f;
	InputVars.AvgRandSpikeFreq = 1.0;

	InputVars.MajorTimePeriod = 15000;
	InputVars.MajorOnTime     = 1000;
	InputVars.MinorTimePeriod = 100;
	InputVars.NoOfNeurons     = 60;

	// Taking input for Optional Simulation Algorithm Parameters
	getInputfromStruct<float>(MatlabInput, "Iext.IRandDecayFactor", InputVars.IRandDecayFactor);
	getInputfromStruct<float>(MatlabInput, "Iext.IRandAmplitude"  , InputVars.IRandAmplitude  );
	getInputfromStruct<float>(MatlabInput, "Iext.IExtAmplitude"   , InputVars.IExtAmplitude   );
	getInputfromStruct<float>(MatlabInput, "Iext.AvgRandSpikeFreq", InputVars.AvgRandSpikeFreq);

	getInputfromStruct<uint32_t>(MatlabInput, "Iext.MajorTimePeriod", InputVars.MajorTimePeriod);
	getInputfromStruct<uint32_t>(MatlabInput, "Iext.MajorOnTime"    , InputVars.MajorOnTime    );
	getInputfromStruct<uint32_t>(MatlabInput, "Iext.MinorTimePeriod", InputVars.MinorTimePeriod);
	getInputfromStruct<uint32_t>(MatlabInput, "Iext.NoOfNeurons"    , InputVars.NoOfNeurons    );

	// Input Validation
	if (InputVars.AvgRandSpikeFreq > 10 || InputVars.AvgRandSpikeFreq < 0)
		WriteException(ExOps::EXCEPTION_INVALID_INPUT, 
			"Iext.AvgRandSpikeFreq is supposed to be between 0 to 10, it is currently %f\n", InputVars.AvgRandSpikeFreq);
	if (InputVars.MajorOnTime > InputVars.MajorTimePeriod)
		WriteException(ExOps::EXCEPTION_INVALID_INPUT, 
			"Iext.MajorOnTime is supposed to be in 0..Iext.MajorTimePeriod (=%d), it is currently %d\n", InputVars.MajorTimePeriod, InputVars.MajorOnTime);
	if (InputVars.MinorTimePeriod > InputVars.MajorOnTime)
		WriteException(ExOps::EXCEPTION_INVALID_INPUT, 
			"Iext.MinorTimePeriod is supposed to be between 0 to Iext.MajorOnTime (=%d), it is currently %d\n", InputVars.MajorOnTime, InputVars.MinorTimePeriod);
	if (InputVars.NoOfNeurons > InputVars.MinorTimePeriod)
		WriteException(ExOps::EXCEPTION_INVALID_INPUT, 
			"Iext.NoOfNeurons is supposed to be between 0 to Iext.MinorTimePeriod (=%d), it is currently %d\n", InputVars.MinorTimePeriod, InputVars.NoOfNeurons);

	// Initializing OutputControl
	// Get OutputControlString and OutputControl Word
	const mxArray* genmxArrayPtr = getValidStructField(MatlabInput, "OutputControl", MexMemInputOps());
	if (genmxArrayPtr != NULL && !mxIsEmpty(genmxArrayPtr)) {
		char * OutputControlSequence = mxArrayToString(genmxArrayPtr);
		InputVars.OutputControl = IExtInterface::getOutputControl(OutputControlSequence);
		mxFree(OutputControlSequence);
	}
}

void IExtInterface::takeInitialStateFromMatlabStruct(
	IExtInterface::SingleStateStruct & IExtInitialStateStruct, 
	const mxArray* IExtMatlabInitState, 
	InputArgs & SimulationInputArgs)
{
	int N = SimulationInputArgs.a.size();
	
	// Initializing Irand
	getInputfromStruct<float>(IExtMatlabInitState, "InitialState.Iext.Iext", IExtInitialStateStruct.Iext, getInputOps(1, "required_size", N));
	
	// Initializing IExtGenState
	{
		bool isNotSingleSeed =
			getInputfromStruct<uint32_t>(IExtMatlabInitState, "InitialState.Iext.IExtGenState", IExtInitialStateStruct.IExtGenState,
				getInputOps(2, "required_size", 1, "no_except"));
		if (isNotSingleSeed)
			getInputfromStruct<uint32_t>(IExtMatlabInitState, "InitialState.Iext.IExtGenState", IExtInitialStateStruct.IExtGenState,
				getInputOps(1, "required_size", 4));
	}

	// Initializing IExtNeuron
	IExtInitialStateStruct.IExtNeuron = 0;
	getInputfromStruct<int>(IExtMatlabInitState, "InitialState.Iext.IExtNeuron", IExtInitialStateStruct.IExtNeuron);

	// Initializing IRandNeuron
	getInputfromStruct<int>(IExtMatlabInitState, "InitialState.Iext.IRandNeuron", IExtInitialStateStruct.IRandNeuron);

}

void IExtInterface::initInternalVariables(
	IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	IExtInterface::InputVarsStruct    & IExtInputVarsStruct, 
	IExtInterface::SingleStateStruct  & IExtInitialStateStruct, 
	InputArgs                         & SimulationInputArgs)
{
	// Aliasing input argument structs
	auto & IntVars = IExtInternalVarsStruct;
	auto & InputVars = IExtInputVarsStruct;
	auto & InitState = IExtInitialStateStruct;
	auto & SimInputArgs = SimulationInputArgs;

	// Initializing Input Vars
	IntVars.IRandDecayFactor = InputVars.IRandDecayFactor;
	IntVars.IRandAmplitude   = InputVars.IRandAmplitude;
	IntVars.IExtAmplitude    = InputVars.IExtAmplitude;
	IntVars.AvgRandSpikeFreq = InputVars.AvgRandSpikeFreq;

	IntVars.MajorTimePeriod  = InputVars.MajorTimePeriod;
	IntVars.MajorOnTime      = InputVars.MajorOnTime    ;
	IntVars.MinorTimePeriod  = InputVars.MinorTimePeriod;
	IntVars.NoOfNeurons      = InputVars.NoOfNeurons    ;

	IntVars.OutputControl = InputVars.OutputControl;

	// ---------- INITIALIZING STATE VARIABLES ---------- //

	// Initializing required constants
	int N = SimInputArgs.a.size();
	int nSteps = SimInputArgs.NoOfms * SimInputArgs.onemsbyTstep;

	// Initializing IExtGen from IExtGenState
	XorShiftPlus::StateStruct RandCurrGenState;

	if (InitState.IExtGenState.size() == 1) {
		IntVars.IExtGen = XorShiftPlus(InitState.IExtGenState[0]);
	}
	else if (InitState.IExtGenState.size() == 4) {
		RandCurrGenState.ConvertVecttoState(InitState.IExtGenState);
		IntVars.IExtGen.setstate(RandCurrGenState);
	}

	// Initializing Iext
	if (InitState.Iext.istrulyempty())
		IntVars.Iext.resize(N, 0.0f);
	else
		IntVars.Iext = InitState.Iext;

	// Initializing IExtNeuron
	IntVars.IExtNeuron = InitState.IExtNeuron;
	
	// Initializing IRandNeuron
	IntVars.IRandNeuron = InitState.IRandNeuron;

}

////////////////////////////////////////////////////////
// Output Struct initialization functions 
////////////////////////////////////////////////////////
void IExtInterface::SingleStateStruct::initialize(
	const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	const InternalVars                      & SimulationInternalVars)
{
	// Aliasing above function parameter structs
	auto & IntVars    = IExtInternalVarsStruct;
	auto & SimIntVars = SimulationInternalVars;
	
	// Initializing State Variables
	IExtGenState = MexVector<uint32_t>(4);
	Iext         = MexVector<float>(SimIntVars.N);
	IExtNeuron   = 0; 
	IRandNeuron  = MexVector<int>(); // Since size is variable (e.g. Input vs Final state)


}

void IExtInterface::StateOutStruct::initialize(
	const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	const InternalVars                      & SimulationInternalVars)
{
	// Aliasing above funtion parameter structs
	auto &IntVars = IExtInternalVarsStruct;
	auto &SimIntVars = SimulationInternalVars;

	// Aliasing some Simulation Vatiables
	auto & StorageStepSize = SimIntVars.StorageStepSize;
	auto & onemsbyTstep = SimIntVars.onemsbyTstep;
	auto & N = SimIntVars.N;
	auto   nSteps = SimIntVars.NoOfms * SimIntVars.onemsbyTstep;

	// Intializing some relevant constants
	int IRandNeuronSize = (int)(IntVars.AvgRandSpikeFreq * 1000 + (SimIntVars.N - 1) + 0.5) / SimIntVars.N; // ceil(round(AvgRandSpikeFreq * 1000)/N)

	// Initializing state variables output struct based on output options
	if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_GEN_STATE_REQ) {
		this->IExtGenStateOut = MexMatrix<uint32_t>(0, 4);
	}
	if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_REQ) {
		this->IextOut = MexMatrix<float>(0, N);
	}
	if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_NEURON_REQ) {
		this->IExtNeuronOut = MexVector<int>(0);
	}
	if (IntVars.OutputControl & IExtInterface::OutOps::I_RAND_NEURON_REQ) {
		this->IRandNeuronOut = MexMatrix<int>(0, IRandNeuronSize);
	}
}

void IExtInterface::OutputVarsStruct::initialize(
	const IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	const InternalVars                      & SimulationInternalVars)
{
	// Aliasing above funtion parameter structs
	auto &IntVars = IExtInternalVarsStruct;
	auto &SimIntVars = SimulationInternalVars;

	// Aliasing some Simulation Vatiables
	auto & StorageStepSize = SimIntVars.StorageStepSize;
	auto & onemsbyTstep    = SimIntVars.onemsbyTstep;
	auto & N               = SimIntVars.N;
	auto   nSteps          = SimIntVars.NoOfms * SimIntVars.onemsbyTstep;

	// Initializing Output Variables according to output options
	// Currently there are no Output variables
}

////////////////////////////////////////////////////////
// C++ Output Functions 
////////////////////////////////////////////////////////
void IExtInterface::doOutput(
	IExtInterface::StateOutStruct     & IExtStateOutStruct,
	IExtInterface::OutputVarsStruct   & IExtOutputVarsStruct,
	IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	InternalVars                      & SimulationInternalVars)
{
	// Aliasing above function parameter structs
	auto &IntVars    = IExtInternalVarsStruct;
	auto &OutVars    = IExtOutputVarsStruct;
	auto &StateOut   = IExtStateOutStruct;
	auto &SimIntVars = SimulationInternalVars;

	// Aliasing some simulation variables
	auto &i               = SimIntVars.i;
	auto &Time            = SimIntVars.Time;
	auto &onemsbyTstep    = SimIntVars.onemsbyTstep;
	auto &StorageStepSize = SimIntVars.StorageStepSize;

	// Initializing some relevant constants

	// Aliasing some IExtInterface variables
	auto &OutputControl = IntVars.OutputControl;

	if (StorageStepSize && (Time % (StorageStepSize*onemsbyTstep) == 0) || !StorageStepSize) {

		// ------------------ OUTPUTTING STATE VARIABLES ------------------ //
		if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_GEN_STATE_REQ) {
			StateOut.IExtGenStateOut.push_row_size(1);
			IntVars.IExtGen.getstate().ConvertStatetoVect(StateOut.IExtGenStateOut.lastRow());
		}
		if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_REQ) {
			StateOut.IextOut.push_row(IntVars.Iext);
		}
		if (IntVars.OutputControl & IExtInterface::OutOps::I_EXT_NEURON_REQ) {
			StateOut.IExtNeuronOut.push_back(IntVars.IExtNeuron);
		}
		if (IntVars.OutputControl & IExtInterface::OutOps::I_RAND_NEURON_REQ) {
			StateOut.IRandNeuronOut.push_row(IntVars.IRandNeuron);
		}
		// ------------------ OUTPUTTING OUTPUT VARIABLES ------------------ //
		// No output variables
	}
}

void IExtInterface::doSingleStateOutput(
	IExtInterface::SingleStateStruct  & IExtSingleStateStruct, 
	IExtInterface::InternalVarsStruct & IExtInternalVarsStruct, 
	InternalVars                      & SimulationInternalVars)
{
	// Aliasing above function parameter structs
	auto &SingleState = IExtSingleStateStruct;
	auto &IntVars     = IExtInternalVarsStruct;
	auto &SimIntVars  = SimulationInternalVars;

	// Aliasing some simulation variables
	auto &i = SimIntVars.i;

	// ------------------ OUTPUTTING STATE VARIABLES ------------------ //

	IntVars.IExtGen.getstate().ConvertStatetoVect(SingleState.IExtGenState);
	SingleState.Iext        = IntVars.Iext;
	SingleState.IExtNeuron  = IntVars.IExtNeuron;
	SingleState.IRandNeuron = IntVars.IRandNeuron;
}

void IExtInterface::doInputVarsOutput(
	IExtInterface::InputVarsStruct    & IExtInInputVarsStruct, 
	IExtInterface::InternalVarsStruct & IExtInternalVarsStruct, 
	InternalVars                      & SimulationInternalVars)
{
	// Aliasing above function parameter structs
	auto & InputVars  = IExtInInputVarsStruct;
	auto & IntVars    = IExtInternalVarsStruct;
	auto & SimIntVars = SimulationInternalVars;

	// Assigning the Input Variables
	InputVars.IRandDecayFactor = IntVars.IRandDecayFactor;
	InputVars.IRandAmplitude   = IntVars.IRandAmplitude;
	InputVars.IExtAmplitude    = IntVars.IExtAmplitude;
	InputVars.AvgRandSpikeFreq = IntVars.AvgRandSpikeFreq;

	InputVars.MajorTimePeriod  = IntVars.MajorTimePeriod;
	InputVars.MajorOnTime      = IntVars.MajorOnTime    ;
	InputVars.MinorTimePeriod  = IntVars.MinorTimePeriod;
	InputVars.NoOfNeurons      = IntVars.NoOfNeurons    ;

	// Note that OutputControl for IExtInterface is not so much an input variable
	// as an intermediate variable calculated from an input to the original Simu-
	// lation. Thus, this variablewill not be  returned or passed as input to the
	// Simulation function
}

////////////////////////////////////////////////////////
// Functions performing output to MATLAB Array
////////////////////////////////////////////////////////
mxArrayPtr IExtInterface::putSingleStatetoMATLABStruct(IExtInterface::SingleStateStruct & IExtSingleStateStruct)
{
	const char *FieldNames[] = {
		"IExtGenState",
		"Iext"        ,
		"IExtNeuron"  ,
		"IRandNeuron" ,
		nullptr
	};
	
	int NFields = 0;
	for (; FieldNames[NFields] != nullptr; ++NFields);
	mwSize StructArraySize[2] = { 1, 1 };

	mxArray * ReturnPointer = mxCreateStructArray_730(2, StructArraySize, NFields, FieldNames);

	// Aliasing input parameter structs
	auto & SingleState = IExtSingleStateStruct;

	// Performing output of Single State variables
	mxSetField(ReturnPointer, 0, "IExtGenState", assignmxArray(SingleState.IExtGenState));
	mxSetField(ReturnPointer, 0, "Iext"        , assignmxArray(SingleState.Iext        ));
	mxSetField(ReturnPointer, 0, "IExtNeuron"  , assignmxArray<int32_t>(SingleState.IExtNeuron  ));
	mxSetField(ReturnPointer, 0, "IRandNeuron" , assignmxArray(SingleState.IRandNeuron ));

	return ReturnPointer;
}

mxArrayPtr IExtInterface::putInputVarstoMATLABStruct(IExtInterface::InputVarsStruct & IExtInputVarsStruct)
{
	const char *FieldNames[] = {
		"IRandDecayFactor",
		"IRandAmplitude"  ,
		"IExtAmplitude"   ,
		"AvgRandSpikeFreq",
		"MajorTimePeriod" ,
		"MajorOnTime"     ,
		"MinorTimePeriod" ,
		"NoOfNeurons"     ,
		nullptr
	};
	
	int NFields = 0;
	for (; FieldNames[NFields] != nullptr; ++NFields);
	mwSize StructArraySize[2] = { 1, 1 };

	mxArray * ReturnPointer = mxCreateStructArray_730(2, StructArraySize, NFields, FieldNames);

	// Aliasing input parameter structs
	auto & InputVars = IExtInputVarsStruct;
	
	// Performing output of Input variables
	mxSetField(ReturnPointer, 0, "IRandDecayFactor", assignmxArray<float>(InputVars.IRandDecayFactor));
	mxSetField(ReturnPointer, 0, "IRandAmplitude"  , assignmxArray<float>(InputVars.IRandAmplitude  ));
	mxSetField(ReturnPointer, 0, "IExtAmplitude"   , assignmxArray<float>(InputVars.IExtAmplitude   ));
	mxSetField(ReturnPointer, 0, "AvgRandSpikeFreq", assignmxArray<float>(InputVars.AvgRandSpikeFreq));

	mxSetField(ReturnPointer, 0, "MajorTimePeriod" , assignmxArray<uint32_t>(InputVars.MajorTimePeriod ));
	mxSetField(ReturnPointer, 0, "MajorOnTime"     , assignmxArray<uint32_t>(InputVars.MajorOnTime     ));
	mxSetField(ReturnPointer, 0, "MinorTimePeriod" , assignmxArray<uint32_t>(InputVars.MinorTimePeriod ));
	mxSetField(ReturnPointer, 0, "NoOfNeurons"     , assignmxArray<uint32_t>(InputVars.NoOfNeurons     ));

	return ReturnPointer;
}

mxArrayPtr IExtInterface::putStateVarstoMATLABStruct(IExtInterface::StateOutStruct & IExtStateOutStruct)
{
	const char *FieldNames[] = {
		"IExtGenState",
		"Iext"        ,
		"IExtNeuron"  ,
		"IRandNeuron" ,
		nullptr
	};

	int NFields = 0;
	for (; FieldNames[NFields] != nullptr; ++NFields);
	mwSize StructArraySize[2] = { 1, 1 };

	mxArray * ReturnPointer = mxCreateStructArray_730(2, StructArraySize, NFields, FieldNames);

	// Aliasing input parameter structs
	auto & StateVars = IExtStateOutStruct;
	
	// Performing output of Input variables
	mxSetField(ReturnPointer, 0, "IExtGenState", assignmxArray(StateVars.IExtGenStateOut));
	mxSetField(ReturnPointer, 0, "Iext"        , assignmxArray(StateVars.IextOut        ));
	mxSetField(ReturnPointer, 0, "IExtNeuron"  , assignmxArray(StateVars.IExtNeuronOut  ));
	mxSetField(ReturnPointer, 0, "IRandNeuron" , assignmxArray(StateVars.IRandNeuronOut ));

	return ReturnPointer;
}

mxArrayPtr IExtInterface::putOutputVarstoMATLABStruct(IExtInterface::OutputVarsStruct & IExtOutputVarsStruct)
{
	const char *FieldNames[] = {
		nullptr
	};

	int NFields = 0;
	for (; FieldNames[NFields] != nullptr; ++NFields);
	mwSize StructArraySize[2] = { 1, 1 };

	mxArray * ReturnPointer = mxCreateStructArray_730(2, StructArraySize, NFields, FieldNames);

	// Aliasing input parameter structs
	auto & OutputVars = IExtOutputVarsStruct;

	// Performing output of Input variables
	// No output variables

	return ReturnPointer;
}

////////////////////////////////////////////////////////
// Iext calculation and update stage 
////////////////////////////////////////////////////////
void IExtInterface::updateIExt(
	IExtInterface::InternalVarsStruct & IExtInternalVarsStruct,
	InternalVars                      & SimulationInternalVars)
{
	// Aliasing above function parameter structs
	auto &IntVars = IExtInternalVarsStruct;
	auto &SimIntVars = SimulationInternalVars;

	// Initializing Constants
	auto &N            = SimIntVars.N;
	auto &Time         = SimIntVars.Time;
	auto &onemsbyTstep = SimIntVars.onemsbyTstep;

	// Resetting IExt
	for (int j = IntVars.IRandNeuron.size(); j > 0; --j) {
		int CurrentRandNeuron = IntVars.IRandNeuron.pop_back();
		if (CurrentRandNeuron > 0)
			IntVars.Iext[CurrentRandNeuron - 1] = 0;
	}	

	if (IntVars.IExtNeuron > 0) {
		IntVars.Iext[IntVars.IExtNeuron - 1] = 0;
		IntVars.IExtNeuron = 0;
		}
	

	// Random Internal Stimulation
	// Added to deliberate external current
	//if (Time  < 100*1000*onemsbyTstep)

	int   NoOfRandomIters   = int(IntVars.AvgRandSpikeFreq*1000 + 0.5)/N;
	float RemainingProbforN = (int(IntVars.AvgRandSpikeFreq*1000 + 0.5) - N*NoOfRandomIters) / (float)N;

	for (int i = 0; i < NoOfRandomIters; ++i) {
		int CurrentRandNeuron = (IntVars.IExtGen() % N) + 1;
		IntVars.IRandNeuron.push_back(CurrentRandNeuron);
		if (IntVars.Iext[CurrentRandNeuron - 1] == 0.0f)
			IntVars.Iext[CurrentRandNeuron - 1] += IntVars.IRandAmplitude;
	}

	if (RemainingProbforN != 0) {
		int CurrentRandNeuron = (IntVars.IExtGen() % (int)(N / RemainingProbforN + 0.5)) + 1;

		if (CurrentRandNeuron <= N) {
			IntVars.IRandNeuron.push_back(CurrentRandNeuron);
			IntVars.Iext[CurrentRandNeuron - 1] += IntVars.IRandAmplitude;
		}
		else {
			IntVars.IRandNeuron.push_back(0);
		}
	}

	// Non-Random External Stimulation
	if (Time % IntVars.MajorTimePeriod < IntVars.MajorOnTime) {
		if (Time % IntVars.MinorTimePeriod < IntVars.NoOfNeurons) {
			IntVars.IExtNeuron = Time % IntVars.MinorTimePeriod + 1;
			IntVars.Iext[IntVars.IExtNeuron - 1] += IntVars.IExtAmplitude;
		}
	}
}
