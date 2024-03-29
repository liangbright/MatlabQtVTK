function Result = vtkplotpoint(FigureHandle, PropName, Point, Color)

Result=[];
%%
Command='vtkplotpoint';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
FigureHandle=num2str(FigureHandle, '&d');

if isempty(PropName)
    PropName='';
end

[~, Num]=size(Point);

PointNum=num2str(int64(Num), '%d');

PointColorValue=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

FileType='raw';
PointDataFileName='PointData.raw';

PointDataType='double';

ResultFileName='Result.json';
%----------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PropName', PropName}, ...
           {'PointNum', PointNum}, ...
           {'PointColorValue', PointColorValue}, ...      
           {'PointDataFileName', PointDataFileName}, ...
           {'PointDataType', PointDataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{PointDataFileName, FileType, PointDataType, Point}};

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

if ~isfield(Result, 'PropHandle')
    Result.PropHandle=[];
end