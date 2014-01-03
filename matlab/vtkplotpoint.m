function PropHandle = vtkplotpoint(FigureHandle, Point, Color)

PropHandle=[];

Client = MatlabClentClass;
%% -----------------------------------------------------------------------
Task={};
Taskhandle=clock;
%% -----------------------------------------------------------------------
IsOK = Client.WriteTask(Task);
if IsOK ~= 1
    return
end

IsOK=Client.InformServer();
if IsOK ~= 1
    return
end

IsOK=Client.CheckTaskStatus();
if IsOK ~= 1
    return
end

Result=Client.ReadResult();

if ~isempty(Result)
    return
end

PropHandle=Result.PropHandle;