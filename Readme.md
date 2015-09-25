## Current Description

1.  The random component of current is a poisson distributed current 
    with adjustable Average frequency between 0-10Hz. Amplitude of 
    this stimulation is non adjustable at 20.0f
2.  The External Current Is a Current spike to the first `NoOfNeurons`
    neurons once Every `MinorTimePeriod` ms for `MajorOnTime` ms every 
	`MinorTimePeriod` ms. Its Amplitude is controllable

## Usage:

It is compatible with all TimeDelNetSim versions which have defined 
the following variables

  `StorageStepSize     - ` Storage Step Size in ms.                              
  `beta                - ` As calculated from Time and StorageStepSize           
  `onemsbyTstep        - ` 1ms/TimeStep (integer)                                
  `N                   - ` Number of Neurons                                     
  `NoOfms              - ` No of ms to simulate for                              
  `i                   - ` Loop Variable.                                        
  `Time                - ` Simulation Time instant as no. of Time Steps since 0 

It is required to be placed in the Headers Directory of Any project so that the 
following files are accessible.

    ..\Network.hpp                                         
    ..\NeuronSim.hpp                                       
    ..\..\..\MexMemoryInterfacing\Headers\MexMem.hpp       
    ..\..\..\MexMemoryInterfacing\Headers\GenericMexIO.hpp 
    ..\..\..\RandomNumGen\Headers\FiltRandomTBB.hpp        

MexMemoryInterfacing Should contain all the generic IO functions, 
pop_back, vWriteOutput, and WriteException functions. Thus it must 
be any version later than or equal to 

e066904 Added vWriteOutput and WriteException

## Variables

	| Variable         | Type     | Description
====|==================|==========|=====================================
    |                  |          |
  1.| Iext             | State    | External Current at current time
    |                  |          |
  2.| IExtGenState     | State    | State of Random Number generator 
    |                  |          | at the end of the current interation
    |                  |          |
  3.| IExtNeuron       | State    | The Neuron which received a non-zero
    |                  |          | Stimulation as apart of the external
    |                  |          | pattern
    |                  |          |
  4.| IRandNeuron      | State    | The Neuron(s) which received a non-
    |                  |          | zero stimulation as a part of the
    |                  |          | Random non-specific stimulation
  5.| MajorTimePeriod  | Input    | The Major Time Period of the 
    |                  |          | External Stimulation
    |                  |          |
  6.| MajorOnTime      | Input    | The Amount of Time in the Major 
    |                  |          | Time Period for which the External
    |                  |          | Stimulation is On (<=MajorTimePeriod)
    |                  |          |
  7.| MinorTimePeriod  | Input    | The Time period of a single pattern
    |                  |          | of IExt Stimulation 
    |                  |          | (<=MajorOnTime)
    |                  |          |
  8.| NoOfNeurons      | Input    | The Number of neurons that are
    |                  |          | Stimulated in the Iext pattern
    |                  |          | (<=MinorTimePeriod)
    |                  |          | 
  9.| IRandAmplitude   | Input    | Amplitude of the random stimulation
    |                  |          |
 10.| IRandDecayFactor | Input    | This is not used in the current sit-
    |                  |          | uation
    |                  |          |
 11.| IExtAmplitude    | Input    | Amplitude of External Stimulation
    |                  |          |
 12.| AvgRandSpikeFreq | Input    | Average Frequency of random stimulation
====|==================|==========|====================================