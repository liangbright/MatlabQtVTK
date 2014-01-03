clear Client
Client = MatlabClientClass;
%%
FigureHandle=vtkfigure();
%%
PointData=[0 1 2 3 4 5 6 7 8 9 
           0 1 2 3 4 5 6 7 8 9 
           0 1 2 3 4 5 6 7 8 9];

Taskhandle='1234567890';

Task={{'Command', 'vtkplotpoint'}, {'PointNum', '10'}, ...
      {'PointColor_R', '1'},  {'PointColor_G', '1'}, {'PointColor_B', '1'}, ...      
      {'PointData', 'PointData.data', 'double', PointData}, ...
      {'PointDataType', 'double'},...
      {'ResultFileName', 'Result'}};
  
IsSucess = Client.WriteTask(Taskhandle, Task);
%%
Client.InformServer();