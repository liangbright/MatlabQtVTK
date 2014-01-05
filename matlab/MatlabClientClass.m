classdef MatlabClientClass < handle
    
properties
      
    port = 12345;
    host = 'localhost';       
    
    javaBufferSize = 1024 * 1024;
    defaultTimeout = 500000;    
end

properties(SetAccess=private, GetAccess=private)
    tcp_socket;
    input;
    output;
end
    
methods

function this = MatlabClientClass() 

end
    
function success = InformServer(this)
% tell the server to read new task

success = 0;
try 
    this.tcp_socket = java.net.Socket(this.host, this.port);

    % get a buffered data input stream from the socket
    this.input = java.io.DataInputStream(java.io.BufferedInputStream(this.tcp_socket.getInputStream));
    this.output = java.io.DataOutputStream(this.tcp_socket.getOutputStream);
    success = 1;    
catch err
    beep;           
    disp('ERROR: cannot connect to LocalTaskServer gui');
    disp(err.message);
end

end

function IsSucess = WriteTask(this, Taskhandle, Task)
% Task.Text is cell : {{key, value}...}
% key is string
% value1 is string
% value2 is matrix
% for example:
% {'Commmand", 'vtkplotpoint'}
% {PointNum", "100"}
% {'DataFileFullName', 'Image.data'}
% Task.Data is cell : {{key, value1, value2}...}
% for example:
% {'Image.data', 'double', Data} Data (value2) will be saved as "Image.data", and dataytpe is double(value1)

% Taskhandle is string

% write JsonObject to "M:/PendingTasks/Taskhandle/Task.json"
% write Data to "M:/PendingTasks/Taskhandle/DataFileFullName"
%--------------------------------------------------------------
FilePath=['M:\PendingTasks\' Taskhandle '\'];
mkdir(FilePath)
%--------------------------------------------------------------
FileName=[FilePath 'Task.json'];

IsSucess=0;

fclose('all');

fid = fopen(FileName, 'w');
if fid == -1
    disp('can not open task file')
    return
end

fprintf(fid, '{\n');

Prefix='    ';

TaskText=Task.Text;

ElementNum=length(TaskText);

for n=1:ElementNum
    Element = TaskText{n};
    [~, L]=size(Element);
    if  L == 2      
        if n < ElementNum
            TextLine=[Prefix '"' Element{1} '"' ': ' '"' Element{2} '"' ',\n'];
        else
            TextLine=[Prefix '"' Element{1} '"' ': ' '"' Element{2} '"' '\n'];
        end
        fprintf(fid, TextLine);        
    else
        disp('Wrong Element @WriteTask')
        fclose(fid);
        return
    end
end

fprintf(fid, '}\n');

fclose(fid);
%-------------------------------------------------------
TaskData=Task.Data;

DataElementNum=length(TaskData);

for n=1:DataElementNum
    DataElement = TaskData{n};
    DataFileName=[FilePath DataElement{1}];
    datafid = fopen(DataFileName, 'w');
    if datafid == -1
        disp('can not open data file')
        fclose(datafid);
        return
    end
    fwrite(datafid, DataElement{3}(:), DataElement{2});
    fclose(datafid);
end
fclose('all');

IsSucess=1;
end

function Status=WaitForResult(this, Taskhandle)
% check if there is M:/CompletedTasks/Taskhandle
% wait within 60 seconds

Status=0;

t1=clock;

while 1
    
    listing = dir(['M:/ProcessedTasks/' Taskhandle] );
    if ~isempty(listing)
        Status=1;
        return
    end
    
    t2=clock;    
    year=t2(1)-t1(1);
    month=t2(2)-t1(2);
    day=t2(3)-t1(3);
    hour=t2(4)-t1(4);
    min=t2(5)-t1(5);
    sec=t2(6)-t1(6);
    
    interval=sec+min*60+hour*60*60+day*60*60*60+month*60*60*60*60+year*60*60*60*60*60;
    
    if interval > 60
        break
    end
    
end

end

function Result=ReadResult(this, Taskhandle, FileName)
% read M:/CompletedTasks/Taskhandle/Result.json                        

% Result.IsSucess
% Result.FigureHandle
% Result.PropHandle
%%

Name=['M:/ProcessedTasks/' Taskhandle '/' FileName];
Result = loadjson(Name);

end

end
end