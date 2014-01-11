function Result = vtkshowtrianglemesh(FigureHandle, PropName, Mesh, MeshColorName)

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
FigureHandle=num2str(FigureHandle, '&d');

if isempty(PropName)
    PropName='';
end

DataFileType='raw';

[~, PointNum]=size(MeshFile.Point);
PointNum=num2str(int64(PointNum), '%d');

PointDataFileName='PointData.raw';
PointDataType='double';

TriangleDataFileName='TriangleData.raw';
TriangleDataType='int64';

[~, TriangleNum]=size(MeshFile.Triangle);
TriangleNum=num2str(int64(TriangleNum), '%d');

% change index to start from 0 (c++, vtk), in matlab it starts from 1.
MeshFile.Triangle=MeshFile.Triangle-1;

ResultFileName='Result.json';
%---------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PropName', PropName}, ...
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

if ~isfield(Result, 'FigureHandle')
   Result.FigureHandle=[];
end

if ~isfield(Result, 'PropHandle')
    Result.PropHandle=[];
end