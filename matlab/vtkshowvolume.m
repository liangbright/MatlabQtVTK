function [Handle, Result] = vtkshowvolume(FigureHandle, Volume)

Handle=[];
Result=[];
%%
Command='vtkshowvolume';
Taskhandle=[Command num2str(uint64(100000*rand))];

Task.Command=Command;
Task.Taskhandle=Taskhandle;
%%
ResultFileName='Result.json';

[Ly, Lx, Lz]=size(Volume);

ImageSize=[num2str(int32(Lx), '%d') ',' num2str(int32(Ly), '%d') ',' num2str(int32(Lz), '%d')];

DataRange=[num2str(min(Volume(:)), '%10.10f') ',' num2str(max(Volume(:)), '%10.10f')];

ImageDataFileName='ImageData.image';
FileType='image';
DataType='double';

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'ImageSize', ImageSize}, ...
           {'ImageDataFileName', ImageDataFileName}, ...
           {'DataType', DataType},...
           {'DataRange', DataRange},...
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

Handle.PropHandle=Result.PropHandle;
Handle.FigureHandle=Result.FigureHandle;
