function [PropHandle, Result] = vtkplotpoint(FigureHandle, Point, Color)

PropHandle=[];
Client = MatlabClientClass;
%%
Taskhandle=['vtkplotpoint' num2str(uint64(100000*rand))];

Command='vtkplotpoint';

ResultFileName='Result.json';

[~, Num]=size(Point);

PointNum=num2str(Num, '%d');

PointColor=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

PointDataFileName='PointData.data';
PointDataType=double;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PointNum', PointNum}, ...
           {'PointColor', PointColor}, ...      
           {'PointDataFileName', PointDataFileName}, ...
           {'PointDataType', PointDataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{PointDataFileName, PointDataType, Point}};

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