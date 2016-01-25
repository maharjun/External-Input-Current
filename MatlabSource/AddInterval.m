function [ IExtPattern ] = AddInterval(IExtPattern, ParentIndex, StartOffset, EndOffset, PatternTimePeriod, NeuronPatternIndex )
%ADDINTERVAL Summary of this function goes here
%   Detailed explanation goes here

IExtPattern.StartOffsetArray        (end+1) = StartOffset;
IExtPattern.EndOffsetArray          (end+1) = EndOffset;
IExtPattern.PatternTimePeriodArray  (end+1) = PatternTimePeriod;
IExtPattern.NeuronPatternIndexArray (end+1) = NeuronPatternIndex;
IExtPattern.ParentIndexArray        (end+1) = ParentIndex;

end
