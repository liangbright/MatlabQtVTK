function Result = vtkshowvolume(FigureHandle, Volume, IntensityDisplayRange)

Result=[];
%%
Command='vtkshowvolume';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
if nargin == 2
    DisplayRange=[num2str(min(Volume(:)), '%10.10f') ',' num2str(max(Volume(:)), '%10.10f')];
else
    DisplayRange=[num2str(IntensityDisplayRange(1), '%10.10f') ',' num2str(IntensityDisplayRange(2), '%10.10f')];   
end
%%
[Ly, Lx, Lz]=size(Volume);

% matlab Image(y,x,z) Origin is always (1,1,1) 
Origin=[num2str(1, '%d') ',' num2str(1, '%d') ',' num2str(1, '%d')];

ImageSize=[num2str(int32(Lx), '%d') ',' num2str(int32(Ly), '%d') ',' num2str(int32(Lz), '%d')];

ImageDataFileName='ImageData.image';
FileType='image';

DataType='double';

ResultFileName='Result.json';
%---------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'ImageSize', ImageSize}, ...
           {'Origin', Origin}, ...
           {'ImageDataFileName', ImageDataFileName}, ...
           {'DataType', DataType},...
           {'IntensityDisplayRange', DisplayRange},...
           {'ResultFileName', ResultFileName}};

Task.Data={{ImageDataFileName, FileType, DataType, Volume}};

%%
Client = MatlabClientClass;

IsSucess = Client.WriteTask(Task);
if IsSucess == 0
    return
end
%%
IsInformed=Client.InformServer();
if IsInformed == 0
    return
end
%%
Status=Client.WaitForResult(Taskhandle);
if Status == 0
    return
end
%%
Result=Client.ReadResult(Taskhandle, ResultFileName);

if ~isfield(Result, 'FigureHandle')
   Result.FigureHandle=[];
end

if ~isfield(Result, 'PropHandle')
    Result.PropHandle=[];
end