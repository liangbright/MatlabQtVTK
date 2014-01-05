function [handle, Result] = vtkfigure()
%%
handle=[];

Client = MatlabClientClass;
%%
Taskhandle=['vtkfigure' num2str(uint64(100000*rand))];

ResultFileName='Result.json';

Task.Text={{'Command', 'vtkfigure'}, {'ResultFileName', ResultFileName}};
Task.Data={};

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

handle=Result.FigureHandle;
