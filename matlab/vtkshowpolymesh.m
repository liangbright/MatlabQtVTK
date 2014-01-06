function [PropHandle, Result] = vtkshowpolymesh(FigureHandle, Mesh)

PropHandle=[];
Client = MatlabClientClass;
%%
Command='vtkshowpolymesh';

Taskhandle=['vtkshowpolymesh' num2str(uint64(100000*rand))];

ResultFileName='Result.json';

PointNum=length(Mesh.Point);
PointNum=num2str(int64(PointNum), '%d');

CellNum=length(Mesh.Cell);
CellNum=num2str(int64(CellNum), '%d');

FileType='txt';

PointDataFileName='PointData.txt';
PointDataType='double';
% convert id to int64
for k=1:length(Mesh.Point)
    temp=Mesh.Point{k};   
    temp{1}=int64(temp{1});
    Mesh.Point{k}=temp;
end

CellDataFileName='CellData.txt';
CellDataType='int64';
% convert all to int64
for k=1:length(Mesh.Cell)
    temp=Mesh.Cell{k};
    num=length(temp);
    for n=1:num
        temp{n}=int64(temp{n});
    end
    Mesh.Cell{k}=temp;
end

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PointNum', PointNum}, ...
           {'CellNum', CellNum}, ...
           {'PointDataFileName', PointDataFileName}, ...
           {'CellDataFileName', CellDataFileName},...
           {'ResultFileName', ResultFileName}};

Task.Data={{PointDataFileName, FileType, PointDataType, Mesh.Point},...
           {CellDataFileName, FileType, CellDataType, Mesh.Cell}};

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

