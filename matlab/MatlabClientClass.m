classdef MatlabClientClass < handle
    
properties
    
    TaskTemplate
        
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
            
    disp('new MatlabClient')
  
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
% Task is cell : {{key, value1, value2}, {key, value1, value2},...}
% key is string
% value1 is string
% value2 is matrix
% for example:
% {"Commmand", "vtkplotpoint"}
% {"PointNum", "100"}
% {"DataFileFullName", "Image.data", ImageData} 
% ImageData (value2) will be saved as "Image.data" (value1)

% Taskhandle is string

% write JsonObject to "M:/PendingTask/Taskhandle/Task.json"
% write Data to "M:/PendingTask/Taskhandle/DataFileFullName"


end

function Status=CheckTaskStatus(this, Taskhandle)
% check if there is M:/CompletedTask/Taskhandle
% wait within 60 seconds

end

function Result=ReadResult(this, Taskhandle)
% read M:/CompletedTask/Taskhandle/Result.json                        

% Result.IsSucess
% Result.FigureHandle
% Result.PropHandle

            
end

end
end