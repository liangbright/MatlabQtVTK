function Result = vtkshowvolume(FigureHandle, PropName, Volume, Origin, Spacing, IntensityDisplayRange)

Result=[];
%%
Command='vtkshowvolume';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
if nargin == 3
    Origin=[];
    Spacing=[];
    IntensityDisplayRange=[];
end
if nargin == 4
    Spacing=[];
    IntensityDisplayRange=[];
end
if nargin == 5
    IntensityDisplayRange=[];
end
%%
if isempty(PropName)
    PropName='';
end
%%
if isempty(IntensityDisplayRange)
    DisplayRange=[num2str(min(Volume(:)), '%10.10f') ',' num2str(max(Volume(:)), '%10.10f')];
else
    DisplayRange=[num2str(IntensityDisplayRange(1), '%10.10f') ',' num2str(IntensityDisplayRange(2), '%10.10f')];   
end

% matlab Volume(y+1,x+1,z+1) is the voxel at (x,y,z) 
% Volume(1,1,1) is the voxel at Origin (0,0,0) 
% Origin is (0,0,0) in world coordinate system
if isempty(Origin)
    ImageOrigin='0, 0, 0';
else
    ImageOrigin=[num2str(Origin(1),'%10.10f'), ',',  num2str(Origin(2),'%10.10f'), ',', num2str(Origin(3),'%10.10f')];
end

if isempty(Spacing)
    ImageSpacing='1, 1, 1';
else
    ImageSpacing=[num2str(Spacing(1),'%10.10f'), ',',  num2str(Spacing(2),'%10.10f'), ',', num2str(Spacing(3),'%10.10f')];
end

[Ly, Lx, Lz]=size(Volume);
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
           {'PropName', PropName}, ...
           {'ImageSize', ImageSize}, ...
           {'ImageOrigin', ImageOrigin}, ...
           {'ImageSpacing', ImageSpacing}, ...
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