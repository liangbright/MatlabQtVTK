function Mesh = ReadPolyMeshDataFile_vtk(FullFileName)

Mesh=[];

tempMesh.Point=zeros(3,10000);
tempMesh.PointGlobalIdxList=zeros(1,10000);

tempMesh.Cell={};
tempMesh.CellGlobalIdxList=[];
tempMesh.IndexTypeInCell='Local';

PointCounter=0;

CellCounter=0;

Flag_cell=0;

fid=fopen(FullFileName, 'r');

LoopCounter =0;

while 1

    LoopCounter=LoopCounter+1;
    
    str = fgets(fid);

    if strcmp(str(1), 'P')
        disp('start to read point')
        break
    else
        if (LoopCounter >= 6)
             disp('invalid file')
            return;
        end
    end
end

str = fgets(fid);

while ischar(str)
    
    if Flag_cell == 0
        PointCounter=PointCounter+1;
        temp = textscan(str,'%f');
        temp=temp{1}';
        
        if length(temp) ~= 3
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
                
        tempMesh.Point(:,PointCounter)=temp;
        
        tempMesh.PointGlobalIdxList(PointCounter)=PointCounter-1;
       
    else
        CellCounter=CellCounter+1;
        temp = textscan(str,'%f');
        temp=temp{1}';
        tempMesh.Cell{CellCounter}=temp(2:end)+1; % matlab index starts from 1
        
        tempMesh.CellGlobalIdxList(CellCounter)=CellCounter-1;
    end
        
    str = fgetl(fid);
    
    if Flag_cell == 0 
       
        if strcmp(str(1), ' ')
            while 1
                str = fgetl(fid);
                if ~strcmp(str(1), ' ')
                    break
                end
            end
        end
    
        if strcmp(str(1), 'C')
             disp('start to read cell')
             Flag_cell=1;      
             str = fgetl(fid);               
        end
    
    else
        if strcmp(str(1), 'C')
             break            
        end 
    end
    
end

fclose(fid);

disp(['Pointer Number : ' num2str(PointCounter)])
disp(['Cell Number : ' num2str(CellCounter)])


tempMesh.Point=tempMesh.Point(:,1:PointCounter);
tempMesh.PointGlobalIdxList=tempMesh.PointGlobalIdxList(1:PointCounter);

Mesh=PolyMeshClass;
Mesh.SetMesh(tempMesh);  