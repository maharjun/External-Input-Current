## Current Description

1.  The random component of current is a poisson distributed current 
    with adjustable Average frequency between 0-10Hz. Amplitude of 
    this stimulation is non adjustable at 20.0f
2.  The External Current Is a Current spike to the first 60 neurons
    once Every 200 ms for 2s every 15s. Its Amplitude is controllable

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

MexMemoryInterfacing Should contain all the generic IO functions. 
it must also have the pop_back method. Thus it must be any version 
later than or equal to 

f56553b Added pop_back method

## Variables

    | Variable         | Type     | Description
====|==================|==========|=====================================
    |                  |          |
    | Iext             | State    | External Current at current time
    |                  |          |
    | IExtGenState     | State    | State of Random Number generator 
    |                  |          | at the end of the current interation
    |                  |          |
    | IExtNeuron       | State    | The Neuron which received a non-zero
    |                  |          | Stimulation as apart of the external
    |                  |          | pattern
    |                  |          |
    | IRandNeuron      | State    | The Neuron(s) which received a non-
    |                  |          | zero stimulation as a part of the
    |                  |          | Random non-specific stimulation
    |                  |          | 
    | IRandAmplitude   | Input    | Amplitude of the random stimulation
    |                  |          |
    | IRandDecayFactor | Input    | This is not used in the current sit-
    |                  |          | uation
    |                  |          |
    | IExtAmplitude    | Input    | Amplitude of External Stimulation
    |                  |          |
    | AvgRandSpikeFreq | Input    | Average Frequency of random stimulation
====|==================|==========|====================================