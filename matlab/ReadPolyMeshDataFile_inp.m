function Mesh = ReadPolyMeshDataFile_inp(FullFileName)

Mesh=[];

tempMesh.Point=zeros(3,10000);
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
            
            tempPoint=zeros(3, PointCounter+1000);
            tempPoint(:,1:PointCounter-1)=tempMesh.Point;
            tempMesh.Point=tempPoint;
        end
                
        tempMesh.Point(:,PointCounter)=temp([2,3,4]);
        
        tempMesh.PointGlobalIdxList(PointCounter)=temp(1);
       
    else
        CellCounter=CellCounter+1;
        temp = textscan(str,'%f,');
        temp=temp{1}';
        tempMesh.Cell{CellCounter}=temp([2,3,4]);     
        
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

[tempMesh, IsOK]=ChangeIndexFromGlobalToLocal(tempMesh);

if IsOK == 1
    Mesh=PolyMeshClass;
    Mesh.SetMesh(tempMesh);  
else
   Mesh=[]; 
end
end

function [Mesh, IsOk]=ChangeIndexFromGlobalToLocal(Mesh)

IsOk=1;

CellNum=length(Mesh.Cell);
CellErrorFlag=zeros(1, CellNum);
for k=1:CellNum

    Cell_k=Mesh.Cell{k};
    
    L_k=length(Cell_k);
        
    for n=1:L_k
        LocalIdx=find(Mesh.PointGlobalIdxList==Cell_k(n));
        Cell_k(n)=LocalIdx;
        if LocalIdx == 0
            disp(['Index is invalid at cell' num2str(k)]);
            CellErrorFlag(k)=1;
            IsOk=0;
        end
    end

    Mesh.Cell{k}=Cell_k;
end

Mesh.Cell=Mesh.Cell(CellErrorFlag==0);

end