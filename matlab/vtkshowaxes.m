function Result = vtkshowaxes(FigureHandle)

Result=[];
%%
Command='vtkshowaxes';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
ResultFileName='Result.json';
%----------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
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

if ~isfield(Result, 'PropHandle')
    Result.PropHandle=[];
end