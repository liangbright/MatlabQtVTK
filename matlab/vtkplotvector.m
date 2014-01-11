function Result = vtkplotvector(FigureHandle, PropName, VectorData, Color)
% plot each Vector=VectorData([1,2,3],n) at Position=VectorData([4,5,6],n) in Color
% 
Result=[];
%%
Command='vtkplotvector';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
FigureHandle=num2str(FigureHandle, '&d');

if isempty(PropName)
    PropName='';
end

[~, VectorNum]=size(VectorData);
VectorNum=num2str(VectorNum, '%d');

VectorColorValue=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

FileType='raw';
VectorDataFileName='VectorData.raw';

VectorDataType='double';

ResultFileName='Result.json';
%----------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PropName', PropName}, ...
           {'VectorNum', VectorNum}, ...
           {'VectorColorValue', VectorColorValue}, ...      
           {'VectorDataFileName', VectorDataFileName}, ...
           {'VectorDataType', VectorDataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{VectorDataFileName, FileType, VectorDataType, VectorData}};

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