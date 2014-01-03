function IsSucess=CellToJson(FileName, Task)

IsSucess=0;

fid = fopen(FileName, 'w');
if fid == -1
    return
end

fprintf(fid, '{\n');

for n=1:length(Task)
    Element = Task(n);
    L=length(Element);
    if  L == 2      
        TextLine=[Element{1} ':' Element{2} '\n'];
        fprintf(fid, TextLine);        
    elseif L == 3
        TextLine=[Element{1} ':' Element{2} '\n'];
        fprintf(fid, TextLine);  
        fid = fopen(Element{2}, 'w');
        fwrite
        
    else
        disp('Wrong Element @CellToJson')
    end
end

fprintf(fid, '}\n');

IsSucess=1;