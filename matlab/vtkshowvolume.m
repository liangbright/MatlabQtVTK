function [PropHandle, Result] = vtkshowvolume(FigureHandle, Volume)

PropHandle=[];
Client = MatlabClientClass;
%%
Command='vtkshowvolume';

Taskhandle=['vtkshowvolume' num2str(uint64(100000*rand))];

ResultFileName='Result.json';

[Lz, Ly, Lx]=size(Volume);
ImageSize=[num2str(int32(Lx), '%d') ',' num2str(int32(Ly), '%d') ',' num2str(int32(Lz), '%d')];

DataRange=[num2str(min(Volume(:)), '%f') ',' num2str(max(Volume(:)), '%f')];

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

IsSucess = Client.WriteTask(Taskhandle, Task);
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

PropHandle=Result.PropHandle;

