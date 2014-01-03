function handle = vtkfigure()
%%
handle=[];

Client = MatlabClientClass;
%%
Taskhandle=['vtkfigure' num2str(uint64(100000*rand))];

ResultFileName='Result.json';

Task={{'Command', 'vtkfigure'}, {'ResultFileName', ResultFileName}};
  
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
Status=Client.CheckTaskStatus(Taskhandle);
if Status == 0
    return
end
%%
Result=Client.ReadResult(Taskhandle, ResultFileName);

handle=Result.FigureHandle;
