function Result = vtkshowsliceofvolume(FigureHandle, VolumeHandle, Plane, IntensityDisplayRange)
% Plane.Origin=[x1,y1,z1]
% Plane.Normal=[x2,y2,z2]
% matlab Image(y+1,x+1,z+1) is the voxel at (x,y,z) 
% Image(1,1,1) is the Origin (0,0,0) 
% Origin is (0,0,0)

Result=[];
%%
Command='vtkshowsliceofvolume';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
DisplayRange=[num2str(IntensityDisplayRange(1), '%10.10f') ',' num2str(IntensityDisplayRange(2), '%10.10f')];   

%origin, normal;
SlicePlane=[num2str(Plane.Origin(1),'%10.10f'), ',',  num2str(Plane.Origin(2),'%10.10f'), ',', num2str(Plane.Origin(3),'%10.10f'), ',', ...
            num2str(Plane.Normal(1),'%10.10f'), ',',  num2str(Plane.Normal(2),'%10.10f'), ',', num2str(Plane.Normal(3),'%10.10f')];    
        
ResultFileName='Result.json';       
%---------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'VolumeHandle', VolumeHandle}, ...
           {'SlicePlane', SlicePlane}, ...
           {'IntensityDisplayRange', DisplayRange},...
           {'ResultFileName', ResultFileName}};

Task.Data={};
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

if isfield(Result, 'PropHandle')
    Result.PropHandle=[];
end