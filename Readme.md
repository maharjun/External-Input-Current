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

However, one may require to update the Visual Studio Project file
to include the new header and cpp files to the project.

It is required to be placed in the Headers Directory of Any project so 
that the following files are accessible.

    ..\Network.hpp                                         
    ..\NeuronSim.hpp                                       
    ..\..\..\MexMemoryInterfacing\Headers\MexMem.hpp       
    ..\..\..\MexMemoryInterfacing\Headers\GenericMexIO.hpp 
    ..\..\..\RandomNumGen\Headers\FiltRandomTBB.hpp        

MexMemoryInterfacing Should contain all the generic IO functions, 
pop_back, vWriteOutput, and WriteException functions. It must also
contain initialization using Initializer list. Thus it must 
be any version later than or equal to 

    edb61a3 Added Constructor by Initialization List for MexVector

## IExtPattern Specification

The IExt Pattern is specified by a Pattern Tree as follows. The
pattern tree is a tree of nested `time intervals`. Each time interval
is specified by 4 unsigned integers. The deepest (leaf) interval links
to a neuron pattern that is to be played.

    1. StartOffset
    2. EndOffset
    3. PatternTimePeriod
    4. NeuronPatternIndex

In addition to the above, associated to each interval is its `ParentIndex`,
i.e. the index of its parent interval.

Note that this scheme requires the following structure fields from 
MATLAB :

    IExtPattern.StartOffsetArray        - uint32 vector
    IExtPattern.EndOffsetArray          - uint32 vector
    IExtPattern.PatternTimePeriodArray  - uint32 vector
    IExtPattern.NeuronPatternIndexArray - uint32 vector
    IExtPattern.ParentIndexArray        - uint32 vector
    
    NeuronPatterns                      - cell of uint32 vectors

The scheme is best described with an example.

### Basic Example ###
consider the following pattern:

    from 0 to inf, every 15000 ms
        from 0 to 1000, every 100 ms
            from 0 to end, execute Neuron Pattern 1
    
    NeuronPattern: 1-60

The above pattern will be specified by the following tree.

    (0, 0   , 15000, 0) -> Parent 0
    (0, 1000, 100  , 0) -> Parent 1
    (0, 0   , 0    , 1) -> Parent 2

for which we will have the following arrays as input

    StartOffsetArray        = [0     0    0];
    EndOffsetArray          = [0     1000 0];
    PatternTimePeriodArray  = [15000 100  0];
    NeuronPatternIndexArray = [0     0    1];
    ParentIndexArray        = [0     1    2];
    
    NeuronPatterns          = {[1,60]};

The NeuronPatterns will be represented by the following cell
array

    {[1,60], ...}

where Neuron Pattern 1 is `[1,60]`

### Multiple Sub-Intervals ###

The above sheme can also have multiple time intervals as the child 
of the same interval level as long as their [StartOffset, EndOffset) 
dont overlap. for example the following pattern 

    from 0 to inf, every 15000 ms
        from 0 to 1000, every 100 ms
            from 0 to end, execute Neuron Pattern 1
        from 2000 to 3000 every 200 ms
            from 0 to end execute Neuron Pattern 2
    
    NeuronPatterns: <length 2 cell array>

The tree would be as follows:

    1: (0, 0   , 15000, 0) -> Parent 0
    2: (0, 1000, 100  , 0) -> Parent 1
    3: (0, 0   , 0    , 1) -> Parent 2
    4: (0, 0   , 200  , 0) -> Parent 1
    5: (0, 1000, 0    , 2) -> Parent 4

### Finite length intervals 

    from 0 to 120000, every 15000 ms
        from 0 to 1000, every 100 ms
            from 0 to end, execute Neuron Pattern 1
        from 2000 to 3000 every 200 ms
            from 0 to end execute Neuron Pattern 2
    from 120000 to inf every 10000 ms
        from 0 to 800, every 80 ms
            from 0 to end, execute Neuron Pattern 3
            
    NeuronPatterns: <length 3 cell array>
    
The above corresponds to the following tree:

    1: (0     , 120000, 15000, 0) -> Parent 0
    2: (0     , 1000  , 100  , 0) -> Parent 1
    3: (0     , 0     , 0    , 1) -> Parent 2
    4: (0     , 0     , 200  , 0) -> Parent 1
    5: (0     , 1000  , 0    , 2) -> Parent 4
    6: (120000, 120000, 10000, 0) -> Parent 0
    7: (0     , 800   , 80   , 2) -> Parent 6
    8: (0     , 0     , 0    , 3) -> Parent 7

### Neuron Pattern Specification

The NeuronPattern is a vector that has the following pattern

sequence of either:

1.  Ranges: 
    
    specified by two consecutive numbers representing RangeStart 
    and RangeEnd (inclusive). The range can be either increasing 
    or decreasing
    
2.  String of Individual Neurons:
    
    specified by a sequence of numbers denoting the neuron to be 
    injected enclosed within an opening and closing 0. If there 
    is no closing 0, all numbers specified after the opening 0 are 
    treated as Individual neurons and not ranges.

e.g. 

    1  60               - 1...60
    1  60 0  3  4  5  0 - 1...60,3,4,5
    30 2  0  1  2  5    - 30...2,1,2,5
    30 2  5  10 0  1  2 - 30...2,5...10,1,2

### MATLAB Helper functions 

The function AddInterval has been added inside the MatlabSource folder.
This function, takes a tree (an IExtPattern struct instance), takes the
details of the interval to be added as arguments, and returns the new tree.
Hopefully, given the nature of the function, these operations will be done
in-place.

## Variables


```
    | Variable                | Type     | Description
====|=========================|==========|=====================================
    |                         |          |
  1.| Iext                    | State    | External Current at current time
    |                         |          |
  2.| IExtGenState            | State    | State of Random Number generator 
    |                         |          | at the end of the current interation
    |                         |          |
  3.| IExtNeuron              | State    | The Neuron which received a non-zero
    |                         |          | Stimulation as apart of the external
    |                         |          | pattern
    |                         |          |
  4.| IRandNeuron             | State    | The Neuron(s) which received a non-
    |                         |          | zero stimulation as a part of the
    |                         |          | Random non-specific stimulation
    |                         |          | 
  5.| StartOffsetArray        | Input    | Array of StartOffset of the Time 
    |                         |          | Intervals
    |                         |          |
  6.| EndOffsetArray          | Input    | Array of EndOffset of the Time 
    |                         |          | Intervals
    |                         |          |
  7.| PatternTimePeriodArray  | Input    | Array of PatternTimePeriod of the Time 
    |                         |          | Intervals
    |                         |          | 
  8.| NeuronPatternIndexArray | Input    | Array of NeuronPatternIndex of the Time 
    |                         |          | Intervals
    |                         |          | 
  9.| ParentIndexArray        | Input    | Array of ParentIndex of the Time
    |                         |          | Intervals
    |                         |          |
 10.| NoOfNeurons             | Input    | The Number of neurons that are
    |                         |          | Stimulated in the Iext pattern
    |                         |          | (<=MinorTimePeriod)
    |                         |          | 
 11.| IRandAmplitude          | Input    | Amplitude of the random stimulation
    |                         |          |
 12.| IRandDecayFactor        | Input    | This is not used in the current sit-
    |                         |          | uation
    |                         |          |
 13.| IExtAmplitude           | Input    | Amplitude of External Stimulation
    |                         |          |
 14.| AvgRandSpikeFreq        | Input    | Average Frequency of random stimulation
====|=========================|==========|====================================
```