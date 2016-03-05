function [ IExtPattern ] = getIExtPatternFromString( PatternStringArray )
%GETIEXTPATTERNFROMSTRING Summary of this function goes here
%  Detailed explanation goes here

if length(PatternStringArray) == 1
	PatternStringArray = strsplit(PatternStringArray, '\n');
end

% Search for a string with the following pattern
% "<indent spaces>", followed by
% 
% "from<spaces>" FOLLOWED BY
% 
% EITHER "<Number>" 
% OR     ("<Number>" followed by 's') 
% OR     ("<Number>" followed by 'ms') 
% FOLLOWED BY
% 
% EITHER
%     "\s*to" FOLLOWED BY
%     EITHER "Number" 
%     OR     ("Number" follwoed by 's') 
%     OR     ("Number" follwoed by 'ms') 
% OR
%     "\s*onwards"
% FOLLOWED BY
% "\s*," FOLLOWED BY
% 
% EITHER 
%     "Every\s*" FOLLOWED BY
%     EITHER "Number" 
%     OR     ("Number" follwoed by 's') 
%     OR     ("Number" follwoed by 'ms')
% OR
%     "Generate (Pattern)?" followed by
%     EITHER "Number" 
%     OR     ("Number" follwoed by 's') 
%     OR     ("Number" follwoed by 'ms'
% FOLLOWED BY
% "\s*", FOLLOWED BY
% END OF LINE

Level = 0;
LevelIndentString = {};
CurrentLevelParent = zeros(0,1,'uint32');
IExtPattern = getEmptyIExtPattern();

Expression = {
		'^(?<Indent>\s*)'...
		'(?<FromSection>from\s+\d+(s|ms)?)\s+'...
		'(?<ToorOnwards>to\s+\d+(s|ms)?|onwards)(\s*,\s*|\s+)'...
		'(?<EveryorGenerate>every\s+\d+(s|ms)?|generate\s+(pattern\s+)?\d+)'...
 		'\s*$'...
	};
Expression = strjoin(Expression, '');

for i = 1:length(PatternStringArray)
	StringMatch = regexpi(PatternStringArray{i}, Expression, 'names');
	
	% Check if matched
	if ~isempty(StringMatch)
		
		% Process Indent Section (determine level)
		Level = 0;
		if (~isempty(LevelIndentString))
			for j = 1:length(LevelIndentString)
				if (strcmpi(StringMatch.Indent,LevelIndentString{j}))
					Level = j;
					LevelIndentString = LevelIndentString(1:Level);
					CurrentLevelParent = CurrentLevelParent(1:Level);
					break;
				end
			end
			if (Level == 0)
				if ~isempty(regexpi(StringMatch.Indent,strcat('^',LevelIndentString{j},'\s+$')))
					Level = j + 1;
					LevelIndentString{end+1} = StringMatch.Indent;
					CurrentLevelParent(end+1) = length(IExtPattern.StartOffsetArray);
				else
					ME = MException('IExtPatternProc:InvalidInput', 'The Indentation level at line %d is not a valid indentation level.', i);
					throw(ME);
				end
			end
		else
			Level = 1;
			LevelIndentString{end+1} = StringMatch.Indent;
			CurrentLevelParent(end+1) = 0;
		end
		
		% Process From Section
		FromSectionMatch = regexpi(StringMatch.FromSection, 'from\s+(?<StartTime>\d+)(?<StartTimeUnit>s|ms)?', 'names');
		StartTime = uint32(str2double(FromSectionMatch.StartTime));
		if (strcmpi(FromSectionMatch.StartTimeUnit, 's'))
			StartTime = StartTime*1000;
		end
		
		% Process ToorOnwards
		if (~isempty(regexpi(StringMatch.ToorOnwards, '^to.*', 'match')))
			ToSectionMatch = regexpi(StringMatch.ToorOnwards, 'to\s+(?<EndTime>\d+)(?<EndTimeUnit>s|ms)?', 'names');
			EndTime = uint32(str2double(ToSectionMatch.EndTime));
			if strcmpi(ToSectionMatch.EndTimeUnit, 's')
				EndTime = EndTime*1000;
			end
		else
			EndTime = StartTime;
		end
		
		% Process EveryorGenerate
		if (~isempty(regexpi(StringMatch.EveryorGenerate, '^every.*', 'match')))
			EverySectionMatch = regexpi(StringMatch.EveryorGenerate, 'every\s+(?<PatternTimePeriod>\d+)(?<PatternTimePeriodUnit>s|ms)?', 'names');
			PatternTimePeriod = uint32(str2double(EverySectionMatch.PatternTimePeriod));
			if strcmpi(EverySectionMatch.PatternTimePeriodUnit, 's')
				PatternTimePeriod = PatternTimePeriod*1000;
			end
			NeuronPatternIndex = uint32(0);
		else
			GenerateSectionMatch = regexpi(StringMatch.EveryorGenerate, 'generate\s+(pattern\s+)?(?<NeuronPatternIndex>\d+)', 'names');
			PatternTimePeriod = uint32(0);
			NeuronPatternIndex = uint32(str2double(GenerateSectionMatch.NeuronPatternIndex));
		end
		
		% Insert Interval
		IExtPattern = AddInterval(IExtPattern, CurrentLevelParent(end), StartTime, EndTime, PatternTimePeriod, NeuronPatternIndex);
	else
		ME = MException('IExtPatternProc:InvalidInput', 'The line %d is not a valid interval specification', i);
		throw(ME);
	end
end

end