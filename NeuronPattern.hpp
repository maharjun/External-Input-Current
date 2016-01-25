#ifndef NEURON_PATTERN_HPP
#define NEURON_PATTERN_HPP

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
class NeuronPatternClass {

private:
	uint32_t MaxNeuronIndex;
	uint32_t Length;
	
	MexVector<uint32_t> NeuronPatternVect;
	MexVector<uint32_t> NeuronPatternTimingVect;

	void calculateLength();
	void ValidateData();
	void calculateTimingVector();

public:

	class Iterator {
	private:
		const NeuronPatternClass *CurrentPattern;
		uint32_t CurrentIndex;
		uint32_t CurrentTimeInstant;

		uint32_t CalculateIndex(uint32_t CurrentTimeInstant);
		void     increment();
		uint32_t getNeuron();

	public:
		Iterator() {
			CurrentPattern = nullptr;
			CurrentIndex = 0;
			CurrentTimeInstant = 0;
		}
		Iterator(const NeuronPatternClass &CurrentPattern_, uint32_t CurrentTimeInstant_ = 0) {
			CurrentPattern = &CurrentPattern_;
			CurrentTimeInstant = CurrentTimeInstant_;
			CurrentIndex = CalculateIndex(CurrentTimeInstant);
		}
		inline void initialize(const NeuronPatternClass &CurrentPattern_, uint32_t CurrentTimeInstant_ = 0) {
			CurrentPattern = &CurrentPattern_;
			CurrentTimeInstant = CurrentTimeInstant;
			CurrentIndex = CalculateIndex(CurrentTimeInstant);
		}
		
		inline NeuronPatternClass::Iterator operator++() {
			this->increment();
			return *this;
		}
		inline NeuronPatternClass::Iterator operator++(int) {
			auto CurrentObject = *this;
			this->increment();
			return CurrentObject;
		}
		uint32_t operator*() {
			return getNeuron();
		}
	};
	NeuronPatternClass() :
		MaxNeuronIndex(0),
		NeuronPatternVect(),
		NeuronPatternTimingVect() {}

	NeuronPatternClass(uint32_t MaxNeuronIndex_, const MexVector<uint32_t> &NeuronPatternVect_) : NeuronPatternClass()
	{
		this->assign(MaxNeuronIndex_, NeuronPatternVect_);
	}

	NeuronPatternClass(uint32_t MaxNeuronIndex, MexVector<uint32_t> &&NeuronPatternVect_) : NeuronPatternClass()
	{
		this->assign(MaxNeuronIndex, std::move(NeuronPatternVect_));
	}

	void assign(uint32_t MaxNeuronIndex_, const MexVector<uint32_t> &  NeuronPatternVect_);
	void assign(uint32_t MaxNeuronIndex_,       MexVector<uint32_t> && NeuronPatternVect_);
	inline uint32_t getLength() const {
		return Length;
	}
	inline NeuronPatternClass::Iterator getIterator(uint32_t RelativeTimeInstant) const {
		return NeuronPatternClass::Iterator(*this, RelativeTimeInstant);
	}
	inline MexVector<uint32_t> getNeuronPatternVect() const {
		return NeuronPatternVect;
	}
};

}

#endif