function PropHandle = vtkplotpoint(FigureHandle, Point, Color)

PropHandle=[];

Client = MatlabClientClass;
%%
Taskhandle=['vtkplotpoint' num2str(uint64(100000*rand))];

ResultFileName='Result.json';

[~, PointNum]=size(Point);

PointColor_R=num2str(Color(1));
PointColor_G=num2str(Color(2));
PointColor_B=num2str(Color(3));

Task={{'Command', 'vtkplotpoint'}, ...
      {'FigureHandle', FigureHandle}, ...
      {'PointNum', num2str(uint64(PointNum))}, ...
      {'PointColor_R', PointColor_R},  {'PointColor_G', PointColor_G}, {'PointColor_B', PointColor_B}, ...      
      {'PointData', 'PointData.data', 'double', Point}, ...
      {'PointDataFileName', 'PointData.data'}, ...
      {'PointDataType', 'double'},...
      {'ResultFileName', ResultFileName}};
  
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