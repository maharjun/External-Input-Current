## Current Description

1.  Gives a Band Limited gaussian Random current with controllable 
    Bandwidth and Variance.
2.  Along with a constant External Current of 9.0 at all times.

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
