function [Handle, Result] = vtkshowtrianglemesh(FigureHandle, Mesh, MeshColorName)

Handle=[];
Result=[];

if isa(Mesh, 'TriangleMeshClass')
    MeshFile=Mesh.GetMesh();
else
    MeshFile=Mesh;
    disp('Warning: Input Mesh may not be a TriangleMesh @ vtkshowtrianglemesh')
end
%%
Command='vtkshowtrianglemesh';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
ResultFileName='Result.json';

DataFileType='vector';

[~, PointNum]=size(MeshFile.Point);

PointDataFileName='PointData.vector';
PointDataType='double';

TriangleDataFileName='TriangleData.vector';
TriangleDataType='int64';

[~, TriangleNum]=size(MeshFile.Triangle);

% change index to start from 0 (c++, vtk), in matlab it starts from 1.
MeshFile.Triangle=MeshFile.Triangle-1;

PointNum=num2str(int64(PointNum), '%d');

TriangleNum=num2str(int64(TriangleNum), '%d');

Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'MeshColorName', MeshColorName}, ...
           {'PointNum', PointNum}, ...
           {'PointDataType', PointDataType}, ...
           {'PointDataFileName', PointDataFileName}, ...
           {'TriangleNum', TriangleNum}, ...
           {'TriangleDataFileName', TriangleDataFileName},...
           {'ResultFileName', ResultFileName}};

Task.Data={{PointDataFileName, DataFileType, PointDataType, MeshFile.Point},...
           {TriangleDataFileName, DataFileType, TriangleDataType, MeshFile.Triangle}};

%%       
Client = MatlabClientClass;       
disp('write data ......')
tic
IsSucess = Client.WriteTask(Task);
if IsSucess == 0
    return
end
toc
disp('write data completed')
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
