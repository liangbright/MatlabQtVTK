function Result = vtkplottensor(FigureHandle, PropName, TensorData, Color)
% plot each Vector=VectorData(1:9,n) at Position=VectorData([10,11,12],n) in Color
% 
Result=[];
%%
Command='vtkplottensor';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
if isempty(PropName)
    PropName='';
end

[~, TensorNum]=size(TensorData);
TensorNum=num2str(TensorNum, '%d');

TensorColorValue=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

FileType='raw';
TensorDataFileName='TensorData.raw';

TensorDataType='double';

ResultFileName='Result.json';
%----------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PropName', PropName}, ...
           {'TensorNum', TensorNum}, ...
           {'TensorColorValue', TensorColorValue}, ...      
           {'TensorDataFileName', TensorDataFileName}, ...
           {'TensorDataType', TensorDataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{TensorDataFileName, FileType, TensorDataType, TensorData}};

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