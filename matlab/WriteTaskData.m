function IsSucess = WriteTaskData(TaskData, FilePath)

%{{DataFileName, FileType, DataType, Data}};

IsSucess=0;

FileNum=length(TaskData);

for n=1:FileNum

    File = TaskData{n};
    
    FullFileName=[FilePath File{1}];    
    FileType=File{2};    
    DataType=File{3};    
    Data=File{4};

    switch FileType
        
        case 'txt'
            IsSucess=WriteTaskData_txt(FullFileName, Data);

        case 'vector'
            IsSucess=WriteTaskData_vector(FullFileName, DataType, Data);

        case 'matrix'
            IsSucess=WriteTaskData_matrix(FullFileName, DataType, Data);

        case 'image'
            IsSucess=WriteTaskData_image(FullFileName, DataType, Data);
            
        otherwise
            disp('unknown FileType')
            return
    end
    
end

end

function IsSucess=WriteTaskData_txt(FullFileName, Data)
% Data format:
% Data={[1.234, 2.345, 4.567, 1.112], [1.2, 2.5, 4.7, 2.1]}
% or
% Data=[1.234, 2.345, 4.567, 1.112; 1.2, 2.5, 4.7, 2.1];
% Each item Data{n} will occupy a line in the text file

IsSucess=0;

fid = fopen(FullFileName, 'w');
    
if fid == -1    
    disp('can not open txt file')    
    return        
end

IsCell=iscell(Data);
if IsCell
    LineNum=length(Data);
else
    [~, LineNum]=size(Data);
end

for k=1:LineNum
    
    if IsCell
        Line=Data{k};
    else
        Line=Data(:,k);        
    end
    
    LineLength=length(Line);
    for n=1:LineLength

        temp=num2str(Line(n), '%10.10f');   

        if n ~= LineLength
            fprintf(fid, [temp ', ']);
        else
            fprintf(fid, temp);
        end
    end
    
    fprintf(fid, '\r\n');
end
fclose(fid);

IsSucess=1;
end


function IsSucess=WriteTaskData_vector(FullFileName, DataType, Data)
% each colume of Data is a vector
% output is binary file

IsSucess=0;

fid = fopen(FullFileName, 'w');
    
if fid == -1
    
    disp('can not open feature file')
        
    fclose(fid);
        
    return

end

fwrite(fid, Data(:), DataType);
    
fclose(fid);
    
IsSucess=1;

end


function IsSucess=WriteTaskData_matrix(FullFileName, DataType, Data)
% if Data is cell, then each item Data{n} is a matrix, and will be saved as Data{n}(:)
% output is binary file

IsSucess=0;

if ~iscell(Data)
    disp('Input Data to WriteTaskData_image must be cell')
end

fid = fopen(FullFileName, 'w');
if fid == -1
    disp('can not open matrix file')
    fclose(fid);
    return
end

L=length(Data);

for n=1:L    
    fwrite(fid, Data{n}(:), DataType);    
end
fclose(fid);

IsSucess=1;

end

function IsSucess=WriteTaskData_image(FullFileName, DataType, Data)
% output is binary file

if ~ismatrix(Data)
    disp('Input Data to WriteTaskData_image must be matrix')
end

[Ly, Lx, Lz]=size(Data);

IsSucess=0;

fid = fopen(FullFileName, 'w');
if fid == -1
    disp('can not open feature file')
    fclose(fid);
    return
end

% Data(y,x,z);
% for z-> for y -> for x
for z=1:Lz;
    temp=Data(:,:,z);
    temp=temp';
    fwrite(fid, temp(:), DataType);    
end
fclose(fid);
    
IsSucess=1;

end


