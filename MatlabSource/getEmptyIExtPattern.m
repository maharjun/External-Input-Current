function [ IExtPattern ] = getEmptyIExtPattern()
%GETEMPTYIEXTPATTERN Summary of this function goes here
%   Detailed explanation goes here

IExtPattern.StartOffsetArray        = zeros(0, 1, 'uint32');
IExtPattern.EndOffsetArray          = zeros(0, 1, 'uint32');
IExtPattern.PatternTimePeriodArray  = zeros(0, 1, 'uint32');
IExtPattern.NeuronPatternIndexArray = zeros(0, 1, 'uint32');

IExtPattern.ParentIndexArray        = zeros(0, 1, 'uint32');

IExtPattern.NeuronPatterns          = cell(0,1);

end
