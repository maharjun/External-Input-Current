#ifndef I_EXT_PATTERN_HPP
#define I_EXT_PATTERN_HPP

#include "NeuronPattern.hpp"

#if defined TIME_DEL_NET_SIM_AS_SUB
#define HEADER_PATHS_TDNS ..
#elif !defined HEADER_PATHS_TDNS
#define HEADER_PATHS_TDNS .
#endif

#define SETQUOTE(A) #A

#define SETQUOTE_EXPAND(A) SETQUOTE(A)

#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/MexMemoryInterfacing/Headers/MexMem.hpp)
#include SETQUOTE_EXPAND(../../../HEADER_PATHS_TDNS/RandomNumGen/Headers/FiltRandomTBB.hpp)

#include <utility>
#include <cstdint>

namespace IExtInterface {

struct IExtPatternProcessor;

class IExtPatternProcessor {
public:
	class TimeIntervalSpec;
	class Iterator;

	class TimeIntervalSpec {
	private:
		uint32_t ActualEndOffset;
	public:
		uint32_t StartOffset;
		uint32_t EndOffset;
		uint32_t PatternTimePeriod;
		uint32_t NeuronPatternIndex;

		TimeIntervalSpec() :
			StartOffset(0),
			EndOffset(0),
			PatternTimePeriod(0),
			NeuronPatternIndex(0) {}

		TimeIntervalSpec(const TimeIntervalSpec &TISpecIn) :
			StartOffset(TISpecIn.StartOffset),
			EndOffset(TISpecIn.EndOffset),
			PatternTimePeriod(TISpecIn.PatternTimePeriod),
			NeuronPatternIndex(TISpecIn.NeuronPatternIndex) {}

		TimeIntervalSpec(
			uint32_t StartOffset_,
			uint32_t EndOffset_,
			uint32_t PatternTimePeriod_,
			uint32_t NeuronPatternIndex_ = 0) :
			StartOffset(StartOffset_),
			EndOffset(EndOffset_),
			PatternTimePeriod(PatternTimePeriod_),
			NeuronPatternIndex(NeuronPatternIndex_) {}

		friend class IExtPatternProcessor;
		friend class IExtPatternProcessor::Iterator;
	};

	class Iterator {
	private:
		MexVector<uint32_t> CurrentIntervalCursor;
		NeuronPatternClass::Iterator CurrentNeuronPatternIter;
		uint32_t CurrentTime;
		const IExtPatternProcessor *Parent;

	public:
		Iterator(const IExtPatternProcessor &Parent_) :
			Parent(&Parent_),
			CurrentNeuronPatternIter(),
			CurrentIntervalCursor(),
			CurrentTime(0) {}

		Iterator(const Iterator &Iter) :
			CurrentIntervalCursor(Iter.CurrentIntervalCursor),
			CurrentNeuronPatternIter(Iter.CurrentNeuronPatternIter),
			CurrentTime(Iter.CurrentTime),
			Parent(Iter.Parent) {}

		inline Iterator operator=(const Iterator &Iter_) {
			this->CurrentIntervalCursor = Iter_.CurrentIntervalCursor;
			this->CurrentNeuronPatternIter = Iter_.CurrentNeuronPatternIter;
			this->CurrentTime = Iter_.CurrentTime;
			this->Parent = Iter_.Parent;
		}

		void initialize(uint32_t Time);
		void increment();
		inline uint32_t dereference() {
			return *CurrentNeuronPatternIter;
		}
	};

private:
	MexVector<TimeIntervalSpec>     TimeIntervalSpecArray;
	MexVector<NeuronPatternClass >  NeuronPatterns;
	MexVector<uint32_t>             ParentIndexArray;
	MexVector<uint32_t>             SortedLeafArray;
	MexVector<MexVector<uint32_t> > TopDownTreeStructure; // Top-down structure with [0] representing the root

	uint32_t                        getIntervalStart(int32_t IntervalIndex)                      const; 
	                                // note IntervalIndex is 1 starting

	MexVector<uint32_t>             getIntervalStartArray()                                      const;
	
	MexVector<MexVector<uint32_t> > getLeafHistories(const MexVector<uint32_t> & LeafIndexArray) const;

	MexVector<uint32_t>             getLeafIndexArray()                                          const;

	MexVector<MexVector<uint32_t> > getTopDownTree()                                             const;

	void                            SortLeafIndexArray();
	void                            ExpandEndOffsets();
	bool                            ValidateIExtPattern();
	void                            CrunchData();

public:

	IExtPatternProcessor() :
		TimeIntervalSpecArray    (),
		NeuronPatterns           (),
		ParentIndexArray         (),
		SortedLeafArray          (),
		TopDownTreeStructure     () {}

	IExtPatternProcessor(
		const MexVector<TimeIntervalSpec>    & TimeIntervalSpecArray_,
		const MexVector<NeuronPatternClass > & NeuronPatterns_,
		const MexVector<uint32_t>            & ParentIndexArray_) :
		TimeIntervalSpecArray    (TimeIntervalSpecArray_),
		NeuronPatterns           (NeuronPatterns_),
		ParentIndexArray         (ParentIndexArray_) {}

	void Initialize(
		const MexVector<uint32_t> & StartOffsetArray_,
		const MexVector<uint32_t> & EndOffsetArray_,
		const MexVector<uint32_t> & PatternTimePeriodArray_,
		const MexVector<uint32_t> & NeuronPatternIndexArray_,
		const MexVector<uint32_t> & ParentIndexArray_,
		const uint32_t MaxNeuronIndex,
		const MexVector<MexVector<uint32_t> > &NeuronPatterns_);

	void IExtInterface::IExtPatternProcessor::Initialize(
		const MexVector<TimeIntervalSpec> & TimeIntervalSpecArray_,
		const MexVector<uint32_t> & ParentIndexArray_,
		const uint32_t MaxNeuronIndex,
		const MexVector<NeuronPatternClass > & NeuronPatterns_);

	MexVector<uint32_t>             getStartOffsetArray()        const;
	MexVector<uint32_t>             getEndOffsetArray()          const;
	MexVector<uint32_t>             getPatternTimePeriodArray()  const;
	MexVector<uint32_t>             getNeuronPatternIndexArray() const;
	MexVector<uint32_t>             getParentIndexArray()        const;
	MexVector<MexVector<uint32_t> > getNeuronPatterns()          const;

};


}

#endif