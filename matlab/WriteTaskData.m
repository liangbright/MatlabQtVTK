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
            WriteTaskData_txt(FullFileName, Data);
            
        case 'image'
            WriteTaskData_image(FullFileName, DataType, Data);

        case 'feature'
            WriteTaskData_feature(FullFileName, DataType, Data);
            
        otherwise
            disp('unknown FileType')
            return
    end
    
end

IsSucess=1;

end

function IsSucess=WriteTaskData_txt(FullFileName, Data)
% Data is matlab-cell text
% Data={{1.234, 2.345, 4.567, 1.112}}
IsSucess=0;

fid = fopen(FullFileName, 'w');
    
if fid == -1    
    disp('can not open txt file')    
    return        
end

LineNum=length(Data);
for k=1:LineNum
    Line=Data{k};
    LineLength=length(Line);
    for n=1:LineLength
        temp=Line{n};        
        if isfloat(temp)
            temp=num2str(temp, '%10.10f');
        else isinteger(temp)
            temp=num2str(temp, '%d');
        end
        
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

function IsSucess=WriteTaskData_feature(FullFileName, DataType, Data)
% each colum of Data is a feature vector

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


function IsSucess=WriteTaskData_image(FullFileName, DataType, Data)



end


