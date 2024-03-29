function Result = vtkplotline(FigureHandle, PropName, Line, Color)

Result=[];
%%
Command='vtkplotline';
Taskhandle=[Command num2str(uint64(100000*rand))];
%%
FigureHandle=num2str(FigureHandle, '&d');

if isempty(PropName)
    PropName='';
end

if iscell(Line)
    LineNum=num2str(int32(length(Line)), '%d');
    
    TotalPointNum=0;
    for k=1:LineNum
        
        [NumOfRow, NumofCol]=size(Line{k});
        if NumOfRow ~= 3
            disp('wrong input @ vtkplotline')
            return
        end        
        TotalPointNum=TotalPointNum+NumofCol;
    end
    
    TotalPointNum=num2str(int32(TotalPointNum), '%d');
else
    [NumOfRow, NumofCol]=size(Line);
    if NumOfRow ~= 3
        disp('wrong input @ vtkplotline')
        return
    end
    LineNum=num2str(1, '%d');
    TotalPointNum=num2str(int32(NumofCol), '%d');
end

LineColorValue=[num2str(Color(1), '%f') ',' num2str(Color(2), '%f') ',' num2str(Color(3), '%f')];

FileType='matrix';
PointDataFileName='LineData.matrix';

PointDataType='double';

ResultFileName='Result.json';
%----------------------------------------------------------------------------------
Task.Command=Command;
Task.Taskhandle=Taskhandle;

Task.Text={{'Command', Command}, ...
           {'FigureHandle', FigureHandle}, ...
           {'PropName', PropName}, ...
           {'LineNum', LineNum}, ...
           {'TotalPointNum', TotalPointNum}, ...
           {'LineColorValue', LineColorValue}, ...      
           {'PointDataFileName', PointDataFileName}, ...
           {'PointDataType', PointDataType},...
           {'ResultFileName', ResultFileName}};
       
Task.Data={{PointDataFileName, FileType, PointDataType, Line}};

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