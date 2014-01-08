function ImageData = ReadDIOMVolume(FullPath)

ImageData=[];
%------------------------------------------------------------------    
AllFileName=dir(fullfile([FullPath '*.dcm']));
Num=length(AllFileName);
if Num == 1
    FileName=AllFileName.name;
else
    FileName=cell(1,Num);
    for k=1:Num
        FileName{k}=AllFileName(k).name;
    end
end
%------------------------------------------------------------------        
if iscell(FileName)
    FileName=sort_nat(FileName, 'ascend');
end
%------------------------------------------------------------------    
    
if iscell(FileName)
    ImageNum=length(FileName);
    I=dicomread([FullPath FileName{1}]);
    [Ly, Lx]=size(I);
    ImageData=zeros(Ly,Lx,ImageNum);
    ImageData(:,:,1)=I;
    for k=2:ImageNum        
        ImageData(:,:,k)=dicomread([FullPath, FileName{k}]);        
    end
else
    ImageData=dicomread([FullPath, FileName]);
end