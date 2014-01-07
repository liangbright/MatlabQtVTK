function [Handle, Result] = vtkshowpolymesh(FigureHandle, Mesh)

Handle=[];
Result=[];

if ~isa(Mesh, 'PolyMeshClass')
    disp('Warning: Input Mesh may not be a PolyMesh @ vtkshowpolymesh')
end
%%
Command='vtkshowpolymesh';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
ResultFileName='Result.json';


DataFileType='txt';

[~, PointNum]=size(Mesh.Point);
PointNum=num2str(int64(PointNum), '%d');

PointDataFileName='PointData.txt';
PointDataType='';

CellDataFileName='CellData.txt';
CellDataType='';

CellNum=length(Mesh.Cell);
CellNum=num2str(int64(CellNum), '%d');

Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PointNum', PointNum}, ...
           {'CellNum', CellNum}, ...
           {'PointDataFileName', PointDataFileName}, ...
           {'CellDataFileName', CellDataFileName},...
           {'ResultFileName', ResultFileName}};

Task.Data={{PointDataFileName, DataFileType, PointDataType, Mesh.Point},...
           {CellDataFileName, DataFileType, CellDataType, Mesh.Cell}};

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
