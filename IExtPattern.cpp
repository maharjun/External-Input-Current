#include "IExtCode.hpp"
#include <algorithm>


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

uint32_t IExtInterface::IExtPatternProcessor::getIntervalStart(int32_t IntervalIndex) const {
	uint32_t IntervalStart;
	uint32_t CurrStartOffset = TimeIntervalSpecArray[IntervalIndex - 1].StartOffset;
	auto CurrParent = ParentIndexArray[IntervalIndex - 1];

	if (CurrParent == 0)
		IntervalStart = CurrStartOffset;
	else {
		auto ParentIntervalStart = getIntervalStart(CurrParent);
		IntervalStart = ParentIntervalStart + CurrStartOffset;
	}
	
	return IntervalStart;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getIntervalStartArray() const 
{
	// This is suboptimal as the traversal to root is happenning for each interval 
	// and information is not being reused. But its ok as it is not performance
	// intensive and this offers coding convenience
	
	size_t NElems = ParentIndexArray.size();
	MexVector<uint32_t> IntervalStartArray(NElems);
	for (int j = 0; j < NElems; ++j) {
		IntervalStartArray[j] = getIntervalStart(j + 1);
	}
	return IntervalStartArray;
}

MexVector<MexVector<uint32_t> > IExtInterface::IExtPatternProcessor::getLeafHistories(const MexVector<uint32_t> & LeafIndexArray) const 
{
	size_t NLeaves = LeafIndexArray.size();
	MexVector<MexVector<uint32_t> > LeafHistories(NLeaves);
	
	for (int j = 0; j < NLeaves; ++j) {

		// Traversing upto root to get indices of Time Interval Specs
		uint32_t CurrentIndex = LeafIndexArray[j];                 // 1 starting
		uint32_t CurrentParent = ParentIndexArray[CurrentIndex-1]; // 1 starting
		LeafHistories[j].push_back(CurrentIndex);

		do {
			CurrentIndex = CurrentParent;
			CurrentParent = ParentIndexArray[CurrentIndex - 1];
			LeafHistories[j].push_back(CurrentIndex);
		} while (CurrentParent > 0);

		// Reversing the array so that root is the first element
		auto TempFwdIter = LeafHistories[j].begin();
		auto TempBackIter = LeafHistories[j].end() - 1; // atleast 1 elem guarenteed to exist
		for (; TempFwdIter < TempBackIter; TempFwdIter++, TempBackIter--) {
			auto tempSwapVar = *TempFwdIter;
			*TempFwdIter = *TempBackIter;
			*TempBackIter = tempSwapVar;
		}
	}
	return LeafHistories;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getLeafIndexArray() const 
{
	size_t NElems = ParentIndexArray.size();
	MexVector<uint32_t> ReturnVector;
	MexVector<bool> isLeaf(NElems, true);

	for (uint32_t j = 0; j < NElems; ++j) {
		uint32_t CurrentIntervalIndex = j+1;
		uint32_t CurrentParent = ParentIndexArray[CurrentIntervalIndex-1];
		while (CurrentParent > 0 && isLeaf[CurrentParent-1]) {
			isLeaf[CurrentParent-1] = 0;
			CurrentIntervalIndex = CurrentParent;
			CurrentParent = ParentIndexArray[CurrentIntervalIndex-1];
		}
	}

	for (int j = 0; j < NElems; ++j)
		if (isLeaf[j])
			ReturnVector.push_back(j + 1);

	return ReturnVector;
}

MexVector<MexVector<uint32_t> > IExtInterface::IExtPatternProcessor::getTopDownTree() const
{
	size_t NIntervals = TimeIntervalSpecArray.size();
	size_t NLeaves = SortedLeafArray.size();
	MexVector<MexVector<uint32_t> > TopDownTree(NIntervals+1);

	for (size_t j = 0; j < NLeaves; ++j) {
		auto CurrentIntervalIndex = SortedLeafArray[j];
		while (CurrentIntervalIndex != 0) {
			auto CurrentParentIndex = ParentIndexArray[CurrentIntervalIndex-1];
			if (!TopDownTree[CurrentParentIndex].isempty() && TopDownTree[CurrentParentIndex].last() == CurrentIntervalIndex) {
				break;
			}
			else {
				TopDownTree[CurrentParentIndex].push_back(CurrentIntervalIndex);
				CurrentIntervalIndex = CurrentParentIndex;
			}
		}
	}
	return TopDownTree;
}

void IExtInterface::IExtPatternProcessor::SortLeafIndexArray()
{
	auto IntervalStartArray = getIntervalStartArray();
	
	std::sort(SortedLeafArray.begin(), SortedLeafArray.end(), [&](const uint32_t &A, const uint32_t &B)->bool {
		return IntervalStartArray[A-1] < IntervalStartArray[B-1];
	});
}

void IExtInterface::IExtPatternProcessor::ExpandEndOffsets()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* This function is resopnsible for taking into account the case where            *
	* EndOffset == StartOffset and calculating the ActualEndOffset and setting it.   *
	*                                                                                *
	* This function requires that Validate be called prior to this to ensure that    *
	* there is no case where EndOffset < StartOffset.                                *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	size_t NIntervals = TimeIntervalSpecArray.size();

	for (size_t i = 0; i < NIntervals; ++i) {
		auto &CurrentInterval = TimeIntervalSpecArray[i];
		if (CurrentInterval.StartOffset == CurrentInterval.EndOffset) {
			if (ParentIndexArray[i] != 0)
				CurrentInterval.ActualEndOffset = TimeIntervalSpecArray[ParentIndexArray[i]-1].PatternTimePeriod;
			else
				CurrentInterval.ActualEndOffset = (uint32_t)(0xFFFFFFFF);
		}
		else {
			CurrentInterval.ActualEndOffset = CurrentInterval.EndOffset;
		}
	}
}

bool IExtInterface::IExtPatternProcessor::ValidateIExtPattern()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* The requirements for this function are:                                                    *
	*                                                                                            *
	*   1. SortedLeafArray be set for the Pattern Tree specified by                              *
	*      TimeIntervalSpecArray and ParentIndexArray.                                           *
	*                                                                                            *
	*   2. The NeuronPatterns should have been validated by                                      *
	*      ValidateNeuronPatterns                                                                *
	*                                                                                            *
	* Here, the following validation steps are implemented:                                      *
	*                                                                                            *
	* 1. For each interval check if:                                                             *
	*                                                                                            *
	*      a. The Start Offset <= End Offset                                                     *
	*                                                                                            *
	*      b. If the interval is contained in another (Parent != 0), then its                    *
	*                                                                                            *
	*             0 <= End Offset < Parents Time Period                                          *
	*                                                                                            *
	*      c. Length of current Interval >= Pattern Time Period                                  *
	*                                                                                            *
	*      d. The Time Period of contained pattern > 0                                           *
	*                                                                                            *
	*      e. If == 0 then the Neuron Pattern Index > 0                                          *
	*                                                                                            *
	* 2. The following are the requirement for all leaf intervals                                *
	*                                                                                            *
	*      a. The Pattern TimePeriod must be 0                                                   *
	*      b. The NeuronPatternIndex must be Non-zero and refer to an actual Neuron Pattern      *
	*      c. The Neuron Pattern must have length <= Length of leaf interval                     *
	*                                                                                            *
	*    The above two conditions ensure that all leaf nodes are neuron patterns                 *
	*                                                                                            *
	* 3. To ensure non-overlapping nature of Intervals, Compare Leaves sequentially as follows:  *
	*                                                                                            *
	*      a. Compare histories lexicographically and find the first point of difference         *
	*         (first from the top)                                                               *
	*                                                                                            *
	*      b. Compare the two Intervals (A, B such that A is the ancestor of the "lesser" leaf)  *
	*         that differ at that point, and require them to satisfy the following:              *
	*                                                                                            *
	*             A.ActualEndOffset <= B.StartOffset                                                   *
	*                                                                                            *
	* 4. Ensure the validity of the Neuron Patterns by ensuring that all neuron indices are      *
	*    within range                                                                            *
	*                                                                                            *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	size_t NElems = ParentIndexArray.size();

	// Validating according to 1. (for each interval...)
	for (int j = 0; j < NElems; ++j) {
		auto CurrentInterval = TimeIntervalSpecArray[j];
		auto CurrentParentInd = ParentIndexArray[j];

		// 1. a. The Start Offset <= End Offset
		if (CurrentInterval.StartOffset > CurrentInterval.EndOffset)
			WriteException(ExOps::EXCEPTION_INVALID_INPUT, 
				"Error in Interval Index %d. "
				"StartOffset (%d) must always be less than or equal to EndOffset (%d)\n", 
				j + 1, CurrentInterval.StartOffset, CurrentInterval.EndOffset);

		// 1. b. If the interval is contained in another (Parent != 0), then its
		//	         
		//	         0 <= End Offset < Parents Time Period
		if (CurrentParentInd != 0) {
			auto CurrentParent = TimeIntervalSpecArray[CurrentParentInd - 1];
			if (CurrentInterval.EndOffset > CurrentParent.PatternTimePeriod)
				WriteException(ExOps::EXCEPTION_INVALID_INPUT,
					"Error in Interval Index %d. "
					"An intervals End Offset (%d) must not exceed the Pattern Time Period of its parent interval (%d)\n",
					j + 1, CurrentInterval.EndOffset, CurrentParent.PatternTimePeriod);
		}

		// 1. c. Length of Interval >= Pattern Time Period
		auto LengthOfInterval = CurrentInterval.EndOffset - CurrentInterval.StartOffset;
		if (LengthOfInterval == 0) {
			if (CurrentParentInd != 0) {
				auto CurrentParent = TimeIntervalSpecArray[CurrentParentInd - 1];
				LengthOfInterval = CurrentParent.PatternTimePeriod - CurrentInterval.StartOffset;
			}
		}
		// if LengthOfInterval is still 0 it means that it is infinite
		// so no issue there
		if (LengthOfInterval != 0 && LengthOfInterval < CurrentInterval.PatternTimePeriod) {
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Interval Index %d. "
				"The Time Period of the contained pattern (%d) cannot exceed the length of the current interval (%d)\n",
				j + 1, CurrentInterval.PatternTimePeriod, LengthOfInterval);
		}

		// 1. d. The Time Period of contained pattern > 0
		//
		//	  e. If == 0 then the Neuron Pattern Index > 0
		if (CurrentInterval.PatternTimePeriod == 0 && CurrentInterval.NeuronPatternIndex == 0)
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Interval Index %d. "
				"In the event that PatternTimePeriod is 0, The interval must point to a Neuron Pattern\n",
				j+1);
	}

	// 2. Validating Leaf Nodes
	size_t NLeaves = SortedLeafArray.size();
	
	for (size_t j = 0; j < NLeaves; ++j) {
		auto &CurrentLeafInterval = TimeIntervalSpecArray[SortedLeafArray[j]-1];
		// 2. a. PatternTimePeriod must be 0
		if (CurrentLeafInterval.PatternTimePeriod > 0) {
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Interval Index %d. "
				"A Leaf Node must not have a non-zero Pattern Time Period\n",
				SortedLeafArray[j]);
		}
		// 2. b. It must point to a Neuron Pattern
		if (CurrentLeafInterval.NeuronPatternIndex == 0 || CurrentLeafInterval.NeuronPatternIndex > NeuronPatterns.size()) {
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Interval Index %d. "
				"The Index of the Neuron Pattern (%d) is not valid i.e. does not lie between 1 and %d\n",
				SortedLeafArray[j], CurrentLeafInterval.NeuronPatternIndex, NeuronPatterns.size());
		}
		// 2. c. The Neuron Pattern must have length <= Length of leaf interval
		auto CurrentLeafIntLength = CurrentLeafInterval.EndOffset - CurrentLeafInterval.StartOffset;
		if (CurrentLeafIntLength == 0) {
			if (ParentIndexArray[SortedLeafArray[j]-1] != 0) {
				CurrentLeafIntLength = TimeIntervalSpecArray[ParentIndexArray[SortedLeafArray[j]-1]-1].PatternTimePeriod;
			}
		}
		
		if (CurrentLeafIntLength != 0 && NeuronPatterns[CurrentLeafInterval.NeuronPatternIndex-1].getLength() > CurrentLeafIntLength) {
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Interval Index %d, Neuron Pattern %d, \n"
				"The Length of the Neuron Pattern (%d) cannot exceed the length of the time interval (%d)\n",
				SortedLeafArray[j], CurrentLeafInterval.NeuronPatternIndex, 
				NeuronPatterns[CurrentLeafInterval.NeuronPatternIndex-1].getLength(), CurrentLeafIntLength);
		}
	}

	// 3. Ensure non-overlapping nature
	auto LeafHistories = getLeafHistories(SortedLeafArray);
	for (int j = 0; j < NLeaves - 1; ++j) {
			
		auto &CurrLeafHistory = LeafHistories[j];
		auto &NextLeafHistory = LeafHistories[j+1];

		auto MinLen = (CurrLeafHistory.size() < NextLeafHistory.size()) ? CurrLeafHistory.size() : NextLeafHistory.size();
		
		for (int k = 0; k < MinLen; ++k) {
			if (CurrLeafHistory[k] != NextLeafHistory[k]) {
				auto LesserDiffAncestor = TimeIntervalSpecArray[CurrLeafHistory[k] - 1];
				auto GreaterDiffAncestor = TimeIntervalSpecArray[NextLeafHistory[k] - 1];
					
				uint32_t EffLesserEndOffset = LesserDiffAncestor.EndOffset;
				if (EffLesserEndOffset == LesserDiffAncestor.StartOffset)
					if (k > 0)
						EffLesserEndOffset = TimeIntervalSpecArray[CurrLeafHistory[k - 1]-1].PatternTimePeriod;
					else
						EffLesserEndOffset = (uint32_t)(0xFFFFFFFF);

				if (LesserDiffAncestor.EndOffset > GreaterDiffAncestor.StartOffset)
					WriteException(ExOps::EXCEPTION_INVALID_INPUT,
						"Error in Interval Indices %d, %d. "
						"The Interval Index %d (relative time from %d-%d) overlaps with the Interval %d (relative time from %d-%d)\n",
						CurrLeafHistory[k], NextLeafHistory[k],
						CurrLeafHistory[k], LesserDiffAncestor.StartOffset, LesserDiffAncestor.EndOffset,
						NextLeafHistory[k], GreaterDiffAncestor.StartOffset, GreaterDiffAncestor.EndOffset);
				break;
			}
		}
	}
	
	return false;
}

void IExtInterface::IExtPatternProcessor::CrunchData()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* This Function VALIDATES the data that has been entered in            *
	*                                                                      *
	*   TimeIntervalSpecArray                                              *
	*   NeuronPatterns                                                     *
	*   ParentIndexArray                                                   *
	*                                                                      *
	* And assigns the following variables                                  *
	*                                                                      *
	*   SortedLeafPattern                                                  *
    *   TopDownTreeStructure                                               *
    *   NeuronPatternTimings                                               *
    *                                                                      *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	// Getting and sorting the leaf index array
	SortedLeafArray = getLeafIndexArray();
	SortLeafIndexArray();

	// Expand End Offsets to their actual value;
	ExpandEndOffsets();

	// Validating the Pattern Tree
	ValidateIExtPattern();

	// Creating Top Down Tree Structure
	TopDownTreeStructure = getTopDownTree();
}

void IExtInterface::IExtPatternProcessor::Initialize(
	const MexVector<uint32_t> & StartOffsetArray_, 
	const MexVector<uint32_t> & EndOffsetArray_, 
	const MexVector<uint32_t> & PatternTimePeriodArray_, 
	const MexVector<uint32_t> & NeuronPatternIndexArray_, 
	const MexVector<uint32_t> & ParentIndexArray_, 
	const uint32_t MaxNeuronIndex,
	const MexVector<MexVector<uint32_t> > & NeuronPatterns_)
{

    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    * This function basically puts the above data into the variables       *
    *                                                                      *
    *   TimeIntervalSpecArray                                              *
    *   NeuronPatterns                                                     *
    *   ParentIndexArray                                                   *
    *                                                                      *
    * And performs Data validation and the initializations of              *
    *                                                                      *
    *   SortedLeafPattern                                                  *
	*   TopDownTreeStructure                                               *
	*   NeuronPatternTimings											   *
    *                                                                      *
    * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	// First, we validate the size of the arrays to ascertain if they are 
	// valid.

	if (StartOffsetArray_.size() != EndOffsetArray_.size() ||
		StartOffsetArray_.size() != PatternTimePeriodArray_.size() ||
		StartOffsetArray_.size() != NeuronPatternIndexArray_.size() ||
		StartOffsetArray_.size() != ParentIndexArray_.size()) {
		WriteException(ExOps::EXCEPTION_INVALID_INPUT,
			"The input vectors of         \n"
			"                             \n"
			"    StartOffsetArray,        \n"
			"    EndOffsetArray,          \n"
			"    PatternTimePeriodArray,  \n"
			"    NeuronPatternIndexArray, \n"
			"    ParentIndexArray         \n"
			"                             \n"
			"Must be of the same length   \n");
	}

	// Then, we Insert the data contained into the variables
	// 
	//   TimeIntervalSpecArray
	//   NeuronPatterns
	//   ParentIndexArray

	size_t NTimeIntSpecs = StartOffsetArray_.size();
	TimeIntervalSpecArray.resize(NTimeIntSpecs);
	for (int i = 0; i < NTimeIntSpecs; ++i) TimeIntervalSpecArray[i].StartOffset        = StartOffsetArray_[i];
	for (int i = 0; i < NTimeIntSpecs; ++i) TimeIntervalSpecArray[i].EndOffset          = EndOffsetArray_[i];
	for (int i = 0; i < NTimeIntSpecs; ++i) TimeIntervalSpecArray[i].PatternTimePeriod  = PatternTimePeriodArray_[i];
	for (int i = 0; i < NTimeIntSpecs; ++i) TimeIntervalSpecArray[i].NeuronPatternIndex = NeuronPatternIndexArray_[i];

	ParentIndexArray = ParentIndexArray_;

	// Performing Input (and Validation) of Neuron Patterns
	NeuronPatterns.resize(NeuronPatterns_.size());
	for (int i = 0; i < NeuronPatterns_.size(); ++i) {
		try {
			NeuronPatterns[i].assign(MaxNeuronIndex, NeuronPatterns_[i]);
		}
		catch (ExOps::ExCodes A) {
			if (A == ExOps::EXCEPTION_INVALID_INPUT) {
				WriteException(ExOps::EXCEPTION_INVALID_INPUT,
					"The Neuron Pattern %d (1-Start Indeing) appears to be invalid\n",
					i + 1);
			}
		}
	}
	
	// Now we perform the validation of the data above and the initialization of
	//
	//     SortedLeafPattern
	CrunchData();
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getStartOffsetArray() const
{
	
	size_t NIntervals = TimeIntervalSpecArray.size();
	MexVector<uint32_t> StartOffsetArrayReturn(NIntervals);
	
	for (size_t i = 0; i < NIntervals; ++i)
		StartOffsetArrayReturn[i] = TimeIntervalSpecArray[i].StartOffset;

	return StartOffsetArrayReturn;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getEndOffsetArray() const
{
	size_t NIntervals = TimeIntervalSpecArray.size();
	MexVector<uint32_t> EndOffsetArrayReturn(NIntervals);

	for (size_t i = 0; i < NIntervals; ++i)
		EndOffsetArrayReturn[i] = TimeIntervalSpecArray[i].EndOffset;

	return EndOffsetArrayReturn;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getPatternTimePeriodArray() const
{
	size_t NIntervals = TimeIntervalSpecArray.size();
	MexVector<uint32_t> PatternTimePeriodArrayReturn(NIntervals);

	for (size_t i = 0; i < NIntervals; ++i)
		PatternTimePeriodArrayReturn[i] = TimeIntervalSpecArray[i].PatternTimePeriod;

	return PatternTimePeriodArrayReturn;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getNeuronPatternIndexArray() const
{
	size_t NIntervals = TimeIntervalSpecArray.size();
	MexVector<uint32_t> NauronPatternIndexArrayReturn(NIntervals);

	for (size_t i = 0; i < NIntervals; ++i)
		NauronPatternIndexArrayReturn[i] = TimeIntervalSpecArray[i].NeuronPatternIndex;

	return NauronPatternIndexArrayReturn;
}

MexVector<uint32_t> IExtInterface::IExtPatternProcessor::getParentIndexArray() const
{
	return ParentIndexArray;
}

MexVector<MexVector<uint32_t> > IExtInterface::IExtPatternProcessor::getNeuronPatterns() const
{
	size_t NPatterns = NeuronPatterns.size();
	MexVector<MexVector<uint32_t> > NeuronPatternsReturn(NPatterns);

	for (size_t i = 0; i < NPatterns; ++i) {
		NeuronPatternsReturn[i] = NeuronPatterns[i].getNeuronPatternVect();
	}
	return NeuronPatternsReturn;
}

void IExtInterface::IExtPatternProcessor::Initialize(
	const MexVector<IExtInterface::IExtPatternProcessor::TimeIntervalSpec> & TimeIntervalSpecArray_,
	const MexVector<uint32_t> & ParentIndexArray_,
	const uint32_t MaxNeuronIndex,
	const MexVector<IExtInterface::NeuronPatternClass > & NeuronPatterns_)
{

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* This function basically puts the above data into the variables       *
	*                                                                      *
	*   TimeIntervalSpecArray                                              *
	*   NeuronPatterns                                                     *
	*   ParentIndexArray                                                   *
	*                                                                      *
	* And performs Data validation and the initializations of              *
	*                                                                      *
	*   SortedLeafPattern                                                  *
	*   TopDownTreeStructure                                               *
    *   NeuronPatternTimings                                               *
	*                                                                      *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	// We Insert the data contained into the variables
	// 
	//   TimeIntervalSpecArray
	//   NeuronPatterns
	//   ParentIndexArray

	TimeIntervalSpecArray = TimeIntervalSpecArray_;
	ParentIndexArray = ParentIndexArray_;
	NeuronPatterns = NeuronPatterns_;

	// Now we perform the validation of the data above and the initialization of
	//
	//     SortedLeafPattern
	//     TopDownTreeStructure
	//     NeuronPatternTimings
	
	CrunchData();
}

void IExtInterface::IExtPatternProcessor::Iterator::initialize(uint32_t TimeInstant)
{
	auto & NeuronPatterns        = Parent->NeuronPatterns       ;
	auto & TimeIntervalSpecArray = Parent->TimeIntervalSpecArray;
	auto & ParentIndexArray      = Parent->ParentIndexArray     ;
	auto & TopDownTreeStructure  = Parent->TopDownTreeStructure ;
	
	uint32_t CurrentNodeIndex = 0;
	uint32_t Level = 0;
	CurrentIntervalCursor.resize(0);
	CurrentTime = TimeInstant;

	uint32_t PreviousLevelTime = TimeInstant;
	do {
		// increment level counter;
		Level++;

		// For each level
		// decide which interval the time instant lies in
		// if it lies in any particular interval,
		const auto &CurrentLevelNodeList = TopDownTreeStructure[CurrentNodeIndex];
		std::function<bool(const uint32_t &, const uint32_t &)> TimeIntervalCompareLesser = [&](const uint32_t &Time, const uint32_t &Index)->bool {
			return Time < TimeIntervalSpecArray[Index-1].StartOffset;
		};
		std::function<bool(const uint32_t &, const uint32_t &)> TimeIntervalCompareGreater = [&](const uint32_t &Index, const uint32_t &Time)->bool {
			return Time >= TimeIntervalSpecArray[Index-1].ActualEndOffset;
		};

		// Find the first interval with starting Time > PreviousLevelTime
		auto UpperBound = std::upper_bound(CurrentLevelNodeList.begin(), CurrentLevelNodeList.end(), PreviousLevelTime, TimeIntervalCompareLesser);
		auto LowerBound = std::lower_bound(CurrentLevelNodeList.begin(), CurrentLevelNodeList.end(), PreviousLevelTime, TimeIntervalCompareGreater);
		
		// Check if The above range contains a valid interval
		if (UpperBound != LowerBound) {
			// If so, then push LowerBound to the stack and update CurrentNodeIndex
			CurrentIntervalCursor.push_back(LowerBound - CurrentLevelNodeList.begin());
			CurrentNodeIndex = *LowerBound;

			// See if we can go deeper. then go deeper.
			if (!TopDownTreeStructure[*LowerBound].isempty()) {
				const auto &ChosenInterval = TimeIntervalSpecArray[*LowerBound - 1];
				PreviousLevelTime = (PreviousLevelTime - ChosenInterval.StartOffset) % ChosenInterval.PatternTimePeriod;
			}
			// If not, then we have hit a neuron pattern. 
			// Perform the initialization of the Neuron Iterator
			// The while below guarantees that this will be the last iteration
			else {
				const auto &ChosenInterval = TimeIntervalSpecArray[*LowerBound - 1];
				CurrentNeuronPatternIter = NeuronPatterns[ChosenInterval.NeuronPatternIndex - 1].getIterator(PreviousLevelTime - ChosenInterval.StartOffset);
				
			}
		}
		// If there is no valid interval
		else {
			// Push UpperBound == LowerBound onto the stack and terminate loop.
			
			// Check if UpperBound == Lowerbound == end
			if (LowerBound == CurrentLevelNodeList.end()) {
				if (Level > 1) {
					// Perform push of first element due to cyclic nature of all
					// levels except 1
					CurrentIntervalCursor.push_back(0);
				}
				else {
					// Push the end as there is no cyclic nature at Level 1
					CurrentIntervalCursor.push_back(LowerBound - CurrentLevelNodeList.begin());
				}
			}
			// If not then just push
			else {
				CurrentIntervalCursor.push_back(LowerBound - CurrentLevelNodeList.begin());
			}
			break;
		}
	} while (!TopDownTreeStructure[CurrentNodeIndex].isempty());
}

void IExtInterface::IExtPatternProcessor::Iterator::increment() {
	
	auto & NeuronPatterns        = Parent->NeuronPatterns       ;
	auto & TimeIntervalSpecArray = Parent->TimeIntervalSpecArray;
	auto & ParentIndexArray      = Parent->ParentIndexArray     ;
	auto & TopDownTreeStructure  = Parent->TopDownTreeStructure ;

	CurrentTime++;
	uint32_t Level = 0;

	uint32_t PreviousLevelTime = CurrentTime;
	uint32_t CurrentParentIntervalIndex = 0;

	// We traverse the depth of the tree to see at which level
	// the interval is exceeded
	do {
		Level++;
		auto &CurrentLevelIntervalList = TopDownTreeStructure[CurrentParentIntervalIndex];
		
		if (PreviousLevelTime == 0) {
			// This is to implement the cursor resetting when the time loops back to 0
			// This is the case where the iterator for Level must be reset
			// to 0. Note that this level will be the last level in the 
			// CurrentIntervalCursor because we are not yet officially
			// inside any interval at this level. We wont break after this
			// as we need to process the reset cursor to see if we are within.
			// the interval pointed to by the reset cursor

			// To understand the meaning of the below, look at the comments
			// in the section dealing with what to do in case the time is not
			// in an interval
			CurrentIntervalCursor[Level - 1] = 0;
			CurrentIntervalCursor.resize(Level);
			CurrentNeuronPatternIter = NeuronPatternClass::Iterator();
		}

		if (CurrentIntervalCursor[Level - 1] == CurrentLevelIntervalList.size()) {
			// This corresponds to the special case where all time intervals
			// for a specific level are over and we are waiting for the time to 
			// either loop to 0 or reach infinity (in which case the if condition
			// above this ensures that we wont enter this one)
			break;
		}

		auto CurrentIntervalIndex = CurrentLevelIntervalList[CurrentIntervalCursor[Level-1]];
		auto &CurrentInterval = TimeIntervalSpecArray[CurrentIntervalIndex-1];

		// Check if the time agrees with this interval
		if (CurrentInterval.StartOffset <= PreviousLevelTime && CurrentInterval.ActualEndOffset > PreviousLevelTime) {

			// Check if the interval is a leaf interval
			if (!TopDownTreeStructure[CurrentIntervalIndex].isempty()) {
				// If not, then we (try to) go deeper. In order to go deeper, 
				// we check if the Index in the next level is pushed into the 
				// stack or not and decide what to do accordingly. This is 
				// necessary because when we first enter an Interval, its sub-
				// Interval index will not have be pushed.

				if (Level == CurrentIntervalCursor.size()) {
					// If Next Level Index not pushed,
					// Then push index 0 (i.e. waiting for / within first sub-interval)
					CurrentIntervalCursor.push_back(0);
				}

				// Update the parent for next iteration (to go deeper)
				CurrentParentIntervalIndex = CurrentIntervalIndex;
				// Update the Time for the next level;
				PreviousLevelTime = (PreviousLevelTime - CurrentInterval.StartOffset) % CurrentInterval.PatternTimePeriod;
			}
			else {
				// If Yes, then the interval given by CurrentIntervalIndex is a 
				// neuron pattern and the CurrentNeuronPatternIter must be processed.
				if (PreviousLevelTime == CurrentInterval.StartOffset) {
					// If we are entering a new neuron pattern, initialize
					// the neuron pattern iterator.
					CurrentNeuronPatternIter = NeuronPatterns[CurrentInterval.NeuronPatternIndex-1].getIterator(0);
				}
				else {
					// If this is a continuation of a previous neuron pattern
					// then increment the existing iterator
					++CurrentNeuronPatternIter;
				}
			}
		}
		else {
			// One of two possibilities, 
			// FIRST, Either the increment has caused the time to go out of the current 
			// interval, i.e. the time instant has not looped back and is equal to
			// CurrentInterval.ActualEndOffset 
			// SECOND, Even with the increment the cursor has not been able to reach
			// The indexed interval (as an interval may be indexed if it is next 
			// to come)

			if (PreviousLevelTime == CurrentInterval.ActualEndOffset) {
				// This corresponds to the FIRST case. 

				// In this case, we will have to update the index of the next
				// interval at this level, truncate CurrentIntervalCursor to the
				// current level and rerun the while loop for this level with the 
				// updated iterator
				
				// The increment below will never push it beyond the "one beyond 
				// the end" position as in that position the loop breaks right in
				// the beginning and will never reach here.

				CurrentIntervalCursor[Level - 1]++;
				CurrentIntervalCursor.resize(Level);

				// resetting neuron pattern iterator as it is like the Level 
				// beyond the last level in the iterator CurrentIntervalCursor 
				// and must thus be "discarded" When any Interval gets over. 
				// This is essential in order for the NeuronPatternIterator 
				// to return 0 when not in any pattern.
				CurrentNeuronPatternIter = NeuronPatternClass::Iterator();

				Level--; // This reruns the while loop for this level
				         // Note that CurrentParentIntervalIndex is not updated
				         // for the same reason.
			}
			else {
				// This corresponds to the THIRD case in which ABSOLUTELY NOTHING 
				// needs to be done. The loop will terminate here because 
				// Level = CurrentLevelIntervalList.size() by design since we have not
				// yet entered an interval at this level.
			}
		}
	} while (Level < CurrentIntervalCursor.size());
}


