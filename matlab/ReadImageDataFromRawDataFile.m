function ImageData=ReadImageDataFromRawDataFile(FilePathAndName, DataType, Lx, Ly, Lz)
% output is binary file (*.image)

ImageData=[];

fid = fopen(FilePathAndName, 'r');
if fid == -1
    disp('can not open data file @ ReadImageDataFromRawDataFile')
    return
end

tempData = fread(fid, Lx*Ly*Lz, DataType);

fclose(fid);

tempL=length(tempData);

if tempL ~= Lx*Ly*Lz
    disp('invalid file @ ReadImageDataFromRawDataFile')
    return;
end

tempData=reshape(tempData, [Lx, Ly, Lz]);

ImageData=zeros(Ly, Lx, Lz);

% Data(y,x,z);
% for z-> for y -> for x
for z=1:Lz;
    temp=tempData(:,:,z);
    ImageData(:,:,z)=temp';
end
    
IsSucess=1;