function IsSucess=WriteImageDataToRawDataFile(FullFileName, DataType, ImageData)
% output is binary file (*.image)

if ~isnumeric(ImageData)
    disp('Input Data to WriteImageDataToRawDataFile must be Array')
end

[~, ~, Lz]=size(ImageData);

IsSucess=0;

fid = fopen(FullFileName, 'w');
if fid == -1
    disp('can not open feature file')
    return
end

%dimention (Lx, Ly, Lz) is in the *.json file

% Data(y,x,z);
% for z-> for y -> for x
for z=1:Lz;
    temp=ImageData(:,:,z);
    temp=temp';
    fwrite(fid, temp(:), DataType);    
end
fclose(fid);
    
IsSucess=1;