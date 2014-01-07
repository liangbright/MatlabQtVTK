function [Handle, Result] = vtkplotpoint(FigureHandle, Point, Color)

Handle=[];
%%
Command='vtkplotpoint';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
ResultFileName='Result.json';

[~, Num]=size(Point);

PointNum=num2str(int64(Num), '%d');

PointColor=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

PointDataFileName='PointData.matrix';
FileType='matrix';
DataType='double';

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PointNum', PointNum}, ...
           {'PointColor', PointColor}, ...      
           {'PointDataFileName', PointDataFileName}, ...
           {'DataType', DataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{PointDataFileName, FileType, DataType, Point}};

%%
Client = MatlabClientClass;

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

Handle.PropHandle=Result.PropHandle;
Handle.FigureHandle=Result.FigureHandle;