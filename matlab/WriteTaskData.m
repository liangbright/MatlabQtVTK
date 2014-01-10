function IsSucess = WriteTaskData(TaskData, FilePath)

% TaskData={{DataFileName, FileType, DataType, Data}, ...};
% FilePath='X:/XXX/';

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
            IsSucess=WriteVectorDataTotxt(FullFileName, Data);

        case 'vector'
            IsSucess=WriteVectorDataTobinary(FullFileName, DataType, Data);

        case 'matrix'
            IsSucess=WriteMatrixDataTobinary(FullFileName, DataType, Data);

        case 'image'
            IsSucess=WriteImageDataTobinary(FullFileName, DataType, Data);
            
        otherwise
            disp('unknown FileType')
            return
    end
    
end

end

function IsSucess=WriteVectorDataTotxt(FullFileName, Data)
% Data format:
% case 1:
% Data=[1.234, 2.345, 4.567, 1.112; 1.2, 2.5, 4.7, 2.1];
% Each colume is a vector, and it will occupy a line in the text file
% case 2:
% Data={[1.234, 2.345, 4.567, 1.112], [1.2, 2.5, 4.7, 2.1]}
% Each item Data{n} is a vector, and it will occupy a line in the text file
% Output
% text file (*.txt)

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


function IsSucess=WriteVectorDataTobinary(FullFileName, DataType, Data)
% Data format:
% case 1:
% Data=[1.234, 2.345, 4.567, 1.112;
%       1.2,   2.5,   4.7,   2.1];
% each colume of Data is a vector
% Data is saved as Data(:)
% case 2:
% Data={[1.234, 2.345, 4.567, 1.112], [1.2, 2.5, 4.7, 2.1]}
% Each item=Data{n} is a vector saved as [length(item), item(1), item(2)]
% Data is saved as [[length(Data{1}(:)), Data{1}(:)], [length(Data{2}(:)), Data{2}(:)], ...]
% Output:
% output is binary file (*.vector)

IsSucess=0;

IsCell=iscell(Data);
if IsCell
    VectorNum=length(Data);
end

fid = fopen(FullFileName, 'w');

if fid == -1    
    disp('can not open vector file')        
    fclose(fid);        
    return
end

if ~IsCell
    fwrite(fid, Data(:), DataType);
else
    for k=1:VectorNum 
        Vector=Data{k};
        Vector=Vector(:);
        temp=[length(Vector); Vector];
        fwrite(fid, temp, DataType);
    end
end

fclose(fid);
    
IsSucess=1;

end


function IsSucess=WriteMatrixDataTobinary(FullFileName, DataType, Data)
% Input Data format:
%
% case 1: raw matrix
% Data=[1.234, 2.345, 4.567, 1.112; 
%       1.2,   2.5,   4.7,   2.1];
% Data is saved as Data(:)
%
% case 2: matrix in cell
% Data={[1.234, 2.345, 4.567, 1.112], 
%        [1.2, 2.5; 
%        4.7, 2.1]}
% Each item=Data{n} is a matrix saved as
% [NumOfRow_n, NumOfCol_n]=size(Data{n});
% Data{n} is saved as [NumOfCol_n, NumOfRow_n, Data{n}(:)] 
% Data is saved as [[NumOfCol_1, NumOfRow_1, Data{1}(:)], [NumOfCol_n, NumOfRow_n, Data{2}(:)], ...]
%
% Output:
% output is binary file (*.matrix)

IsSucess=0;

IsCell=iscell(Data);
if IsCell
    MatrixNum=length(Data);
end

fid = fopen(FullFileName, 'w');

if fid == -1    
    disp('can not open vector file')        
    fclose(fid);        
    return
end

if ~IsCell
    fwrite(fid, Data(:), DataType);
else
    for k=1:MatrixNum 
        Matrix=Data{k};
        [NumOfRow, NumOfCol]=size(Matrix);
        temp=[NumOfCol; NumOfRow; Matrix(:)];
        fwrite(fid, temp, DataType);
    end
end

fclose(fid);
    
IsSucess=1;

end

function IsSucess=WriteImageDataTobinary(FullFileName, DataType, Data)
% output is binary file (*.image)

if ~ismatrix(Data)
    disp('Input Data to WriteImageData_binary must be matrix')
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


