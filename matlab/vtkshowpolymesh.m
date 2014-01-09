function Result = vtkshowpolymesh(FigureHandle, Mesh, MeshColorName)

%MeshColorName
% white
% black
% read
% blue
% yellow
% cyan
% magenta
% silver
% orange
% gold
% darkred
% purple
% wheat

Result=[];

if isa(Mesh, 'PolyMeshClass')
    disp('Warning: Input Mesh may not be a PolyMesh @ vtkshowpolymesh')
    MeshFile=Mesh.GetMesh();
else
    MeshFile=Mesh;    
end
%%
Command='vtkshowpolymesh';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
DataFileType='vector';

[~, PointNum]=size(MeshFile.Point);

PointDataFileName='PointData.vector';
PointDataType='double';

CellDataFileName='CellData.vector';
CellDataType='int64';

CellNum=length(MeshFile.Cell);

% change matlab index (start from 0) to  c++/vtk index (start from 0)
for k=1:CellNum
    MeshFile.Cell{k}=MeshFile.Cell{k}-1;
end

CellNum=num2str(int64(CellNum), '%d');

PointNum=num2str(int64(PointNum), '%d');

ResultFileName='Result.json';
%----------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'MeshColorName', MeshColorName}, ...
           {'PointNum', PointNum}, ...          
           {'PointDataFileName', PointDataFileName}, ...
           {'PointDataType', PointDataType}, ...
           {'CellNum', CellNum}, ...
           {'CellDataFileName', CellDataFileName},...
           {'ResultFileName', ResultFileName}};

Task.Data={{PointDataFileName, DataFileType, PointDataType, MeshFile.Point},...
           {CellDataFileName, DataFileType, CellDataType, MeshFile.Cell}};

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
