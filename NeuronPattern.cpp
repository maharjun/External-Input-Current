#include "NeuronPattern.hpp"

#include <MexMemoryInterfacing/Headers/MexMem.hpp>
#include <MexMemoryInterfacing/Headers/GenericMexIO.hpp>
#include <RandomNumGen/Headers/FiltRandomTBB.hpp>

#include <algorithm>
#include <cstdint>

void IExtInterface::NeuronPatternClass::calculateLength()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	*  This function, calculates the length (i.e. the number of timesteps taken    *
	*  by the currently specified pattern). For this function to work correctly,   *
	*  it is necessary for the Data to have been validated (using the ValidateData *
	*  function)                                                                   *
	*                                                                              *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	size_t NeuronPatternSize = NeuronPatternVect.size();
	this->Length = 0;
	
	bool isIndividualNeurons = false;
	// Analyse the current neuron pattern and calculate its length
	for (size_t k = 0; k < NeuronPatternSize;) {

		// Checking for occurrence of 0 toggling the individual/range state 
		if (NeuronPatternVect[k] == 0) {
			isIndividualNeurons = !isIndividualNeurons;
			k++;
		}
		// Taking into account an individual neuron
		else if (isIndividualNeurons) {
			Length++;
			k++;
		}
		// Taking into account ranges
		else if (k < NeuronPatternSize - 1) {
			auto BeginNeuron = NeuronPatternVect[k];
			auto EndNeuron = NeuronPatternVect[k + 1];
			Length += (BeginNeuron < EndNeuron) ? EndNeuron - BeginNeuron + 1 : BeginNeuron - EndNeuron + 1;
			k += 2;
		}
	}
}

void IExtInterface::NeuronPatternClass::ValidateData()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* This function validates the Neuron Patterns as follows:                      *
	*                                                                              *
	*   1. All ranges must be completely specified (start and end)                 *
	*   2. All Neuron Indices should be <= MaxNeuronIndex                          *
	*                                                                              *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	
	size_t CurrentPatternSize = NeuronPatternVect.size();

	// 1. All ranges must be completely specified (start and end)
	// Iterate through the Pattern array
	bool isRangeEndSpecified = true; // This is toggled every time a range beg/end is encountered
	                                 // It represents the state of the range specification
	bool isIndividualNeuron = false; // This flag denotes whether we are currently parsing individual 
	                                 // neurons or neurons representing ranges

	for (size_t k = 0; k < CurrentPatternSize; k++) {
		if (NeuronPatternVect[k] == 0) {
			if (isIndividualNeuron)
				isIndividualNeuron = false;
			else
				if (isRangeEndSpecified)
					isIndividualNeuron = true;
				else
					WriteException(ExOps::EXCEPTION_INVALID_INPUT,
						"Error in Neuron Pattern, before position %d (1-Start Index)\n"
						"Ranges must be specified completely\n",
						k + 1);
		}
		else if (!isIndividualNeuron) {
			isRangeEndSpecified = !isRangeEndSpecified;
		}
	}

	// 2. All Neuron Indices should be <= MaxNeuronIndex
	for (size_t k = 0; k < CurrentPatternSize; ++k) {
		if (NeuronPatternVect[k] > MaxNeuronIndex) {
			WriteException(ExOps::EXCEPTION_INVALID_INPUT,
				"Error in Neuron Pattern at position %d (1-Start Index)\n"
				"The Neuron Index (%d) must not exceed the maximum nauron index (%d)\n",
				k + 1,
				NeuronPatternVect[k], MaxNeuronIndex);
		}
	}

}

void IExtInterface::NeuronPatternClass::calculateTimingVector()
{
	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	* This function calculates the Neuron Pattern Timing Vect. The neuron pattern  *
	* must be validated before this function is called.                            *
	*                                                                              *
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

	size_t PatternSize = NeuronPatternVect.size();
	NeuronPatternTimingVect.resize(PatternSize);

	bool isIndividualNeuron = false;
	uint32_t CurrentTimeOffset = 0;
	for (size_t k = 0; k < PatternSize;) {
		auto CurrentEntry = NeuronPatternVect[k];
		if (CurrentEntry == 0) {
			// Toggling the state between ranges and individual neurons
			// The time assigned to a 0 is the time of occurrence of the 
			// first Neuron after it.
			NeuronPatternTimingVect[k] = CurrentTimeOffset;
			isIndividualNeuron = !isIndividualNeuron;
			k++;
		}
		else {
			if (isIndividualNeuron) {
				// Assigning the time of occurence of an individual neuron
				// and incrementing it
				NeuronPatternTimingVect[k] = CurrentTimeOffset;
				CurrentTimeOffset++;
				k++;
			}
			else {
				// Assigning the time of occurrence of the begin and end of range
				// and incrementing it by the length of the range
				auto BegNeuron = NeuronPatternVect[k];
				auto EndNeuron = NeuronPatternVect[k + 1]; // This should not create an error if Pattern is validated

				uint32_t LengthofSeq = (BegNeuron < EndNeuron) ? EndNeuron - BegNeuron + 1 : BegNeuron - EndNeuron + 1;
				NeuronPatternTimingVect[k] = CurrentTimeOffset;
				NeuronPatternTimingVect[k + 1] = CurrentTimeOffset + LengthofSeq - 1;

				CurrentTimeOffset += LengthofSeq;
				k += 2;
			}
		}
	}
}

void IExtInterface::NeuronPatternClass::assign(uint32_t MaxNeuronIndex_, const MexVector<uint32_t>& NeuronPatternVect_)
{
	this->MaxNeuronIndex = MaxNeuronIndex_;
	this->NeuronPatternVect = NeuronPatternVect_;
	ValidateData();
	calculateLength();
	calculateTimingVector();
}

void IExtInterface::NeuronPatternClass::assign(uint32_t MaxNeuronIndex_, MexVector<uint32_t>&& NeuronPatternVect_)
{
	this->MaxNeuronIndex = MaxNeuronIndex_;
	this->NeuronPatternVect = std::move(NeuronPatternVect_);
	ValidateData();
	calculateLength();
	calculateTimingVector();
}

uint32_t IExtInterface::NeuronPatternClass::Iterator::CalculateIndex(uint32_t CurrentTimeInstant)
{
	if (CurrentTimeInstant >= this->CurrentPattern->Length) {
		// In case the Time instant exceeds the defined time range for 
		// the pattern, we simple set the index to be the end of the array
		return this->CurrentPattern->NeuronPatternVect.size();
	}
	else {
		
		// The return statement is not as simple as it looks. There are multiple cases
		// 
		// CASE I: Time instant is exactly present in array
		//    SUBCASE I: 
		//    
		//       There are zeros on NeuronPatternVect which result in multiple
		//       consecutive occurrences of the same timing. In this case, the
		//       last of them must be considered as it will not be a zero. Hence
		//       UboundIter-1
		//    
		//    SUBCASE II:
		//    
		//       If there is only a single occurrence (e.g. in Individual Neurons 
		//       or at Range Edges) then UBoundIter-1 will correspond to it.
		// 
		// CASE II: In case the time instant is not present
		//    
		//    Given that Time < Length, this can only happen when thhe instant 
		//    corresponds to a time that is in between a range. In this case,
		//    we choose the position indicating the star of the range as the
		//    required index. This will of course be UBoundIter-1

		auto UBoundIter = std::upper_bound(
			this->CurrentPattern->NeuronPatternTimingVect.begin(), this->CurrentPattern->NeuronPatternTimingVect.end(),
			CurrentTimeInstant);

		return (UBoundIter - 1 - this->CurrentPattern->NeuronPatternTimingVect.begin());
	}
	return uint32_t();
}

void IExtInterface::NeuronPatternClass::Iterator::increment()
{
	// This function performs the increment operation for the iterator
	// 
	// First, we increment the time instant
	// Second, we handle the incrementing of the CurrentIndex as follows:
	// 
	// CASE I : Incremented Time instant lies within the range
	// 
	//    As long as the next index points to the incremented 
	//    CurrentTimeInstant, we increment CurrentIndex. This takes care
	//    of the cases where multiple consecutive entries may bear the
	//    same index as a result of 0's in the NeuronPatternVect
	// 
	// CASE II: Incremented Time instant lies outside the range
	//    
	//    In this case, CurrentIndex is assigned the index beyond the 
	//    end of the NeuronPatternVect

	if (CurrentPattern != nullptr) {
		CurrentTimeInstant++;
		if (CurrentTimeInstant < CurrentPattern->Length) {
			while (
				CurrentIndex + 1 < CurrentPattern->NeuronPatternVect.size() &&
				CurrentPattern->NeuronPatternTimingVect[CurrentIndex + 1] == CurrentTimeInstant) {
				CurrentIndex++;
			}
		}
		else {
			CurrentIndex = CurrentPattern->NeuronPatternVect.size();
		}
	}
}

uint32_t IExtInterface::NeuronPatternClass::Iterator::getNeuron()
{
	if (CurrentPattern != nullptr) {

		uint32_t NeuronPatternSize = CurrentPattern->NeuronPatternVect.size();
		if (CurrentIndex < NeuronPatternSize) {

			// For individual Neurons and range Begins AND Ends, we get
			// DistanceFromRangeBeg = 0 for which we will get the neuron index at that
			// location.
			// 
			// If DistanceFromRangeBeg > 0, then we are in the middle of a range
			// we then find out if this is a decreasing or an increasing range and
			// then outputthe apropriate intermediate neuron

			uint32_t DistanceFromRangeBeg = CurrentTimeInstant - CurrentPattern->NeuronPatternTimingVect[CurrentIndex];

			if (DistanceFromRangeBeg == 0)
				return CurrentPattern->NeuronPatternVect[CurrentIndex];
			else
				if (CurrentPattern->NeuronPatternVect[CurrentIndex + 1] > CurrentPattern->NeuronPatternVect[CurrentIndex])
					return CurrentPattern->NeuronPatternVect[CurrentIndex] + DistanceFromRangeBeg;
				else
					return CurrentPattern->NeuronPatternVect[CurrentIndex] - DistanceFromRangeBeg;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}
