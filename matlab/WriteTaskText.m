function IsSucess = WriteTaskText(TaskText, FullFileName)

IsSucess=0;

fid = fopen(FullFileName, 'w');
if fid == -1
    disp('can not open task file')
    return
end

fprintf(fid, '{\n');

Prefix='';

ElementNum=length(TaskText);

for n=1:ElementNum
    Element = TaskText{n};
    [~, L]=size(Element);
    if  L == 2      
        if n < ElementNum
            TextLine=[Prefix '"' Element{1} '"' ': ' '"' Element{2} '"' ',\r\n'];
        else
            TextLine=[Prefix '"' Element{1} '"' ': ' '"' Element{2} '"' '\r\n'];
        end
        fprintf(fid, TextLine);        
    else
        disp('Wrong Element @WriteTaskText')
        fclose(fid);
        return
    end
end

fprintf(fid, '}\n');

fclose(fid);

IsSucess=1;