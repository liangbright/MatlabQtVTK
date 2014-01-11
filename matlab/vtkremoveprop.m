function Result = vtkremoveprop(FigureHandle, PropHandle)
%%
Result=[];

Client = MatlabClientClass;
%%
Command='vtkremoveprop';
Taskhandle=[Command num2str(uint64(100000*rand))];
%---------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

ResultFileName='Result.json';

Task.Text={{'Command', Command},...
           {'FigureHandle', num2str(FigureHandle, '%d')}, ...
           {'PropHandle',   num2str(PropHandle, '%d')}, ...
           {'ResultFileName', ResultFileName}};
           
Task.Data={};

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