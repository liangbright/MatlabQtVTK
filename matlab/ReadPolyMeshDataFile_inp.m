function Mesh = ReadPolyMeshDataFile_inp(FullFileName)

Mesh=[];

tempMesh.Point=zeros(4,10000);
tempMesh.PointGlobalIdxList=zeros(1,10000);

tempMesh.Cell={};
tempMesh.CellGlobalIdxList=[];
tempMesh.IndexTypeInCell='Global';

PointCounter=0;

CellCounter=0;

Flag_cell=0;

fid=fopen(FullFileName, 'r');

str = fgets(fid);

if strcmp(str(1), '*')
    disp('start to read point')
else
    disp('invalid file')
    return;
end

str = fgets(fid);

while ischar(str)
    
    if Flag_cell == 0
        PointCounter=PointCounter+1;
        temp = textscan(str,'%f,');
        temp=temp{1}';
        
        if length(temp) ~= 4
            disp('invalid file - point')
            return
        end
                
        if PointCounter > length(tempMesh.PointGlobalIdxList)
            tempList=zeros(1, PointCounter+1000);
            tempList(1:PointCounter-1)=tempMesh.PointGlobalIdxList;
            tempMesh.PointGlobalIdxList=tempList;
            
            tempPoint=zeros(4, PointCounter+1000);
            tempPoint(:,1:PointCounter-1)=tempMesh.Point;
            tempMesh.Point=tempPoint;
        end
                
        tempMesh.Point(:,PointCounter)=temp;
        
        tempMesh.PointGlobalIdxList(PointCounter)=temp(1);
       
    else
        CellCounter=CellCounter+1;
        temp = textscan(str,'%f,');
        temp=temp{1}';
        tempMesh.Cell{CellCounter}=temp;     
        
        tempMesh.CellGlobalIdxList(CellCounter)=temp(1);
    end
        
    str = fgetl(fid);
    
    if strcmp(str(1), '*')
        
         str = fgetl(fid);
         if strcmp(str(1), '*')
             disp('start to read cell')
             Flag_cell=1;      
             str = fgetl(fid);               
         else
             disp('invalid file')
             fclose(fid);
             return
         end
    end
end

fclose(fid);

tempMesh.Point=tempMesh.Point(:,1:PointCounter);
tempMesh.PointGlobalIdxList=tempMesh.PointGlobalIdxList(1:PointCounter);

Mesh=PolyMeshClass;
Mesh.SetMesh(tempMesh);   