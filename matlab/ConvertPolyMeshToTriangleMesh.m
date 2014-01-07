function TriangleMesh = ConvertPolyMeshToTriangleMesh(PolyMesh)

TriangleMesh=TriangleMeshClass;

TriangleMesh.Point=PolyMesh.Point;

TriangleMesh.PointIDList=PolyMesh.PointPointIDList;

%TriangleMesh.Part_PointIdxTable=PolyMesh.Part_PointIdxTable;

CellCounter=0;

SplitCellIdxList=[];
SplitCounter=0;
NewCellIdxList={};

for k=1:PolyMesh.CellNum
    
    Cell_k=PolyMesh.Cell(k);
    
    L_k=length(Cell_k);
    if  L_k > 4 % not a Triangle
    
        % 1,2,3 -> cell_k_1
        % 3,4,1 -> cell_k_2
    
        tempCell{1}={CellCounter+1  Cell_k(2), Cell_k(3), Cell_k(4)};
        tempCell{2}={CellCounter+2, Cell_k(4), Cell_k(5), Cell_k(2)};
    
       % SplitCounter=SplitCounter+1;
       % SplitCellIdxList(SplitCounter)=Cell_k(1);
       % NewCellIdxList{SplitCounter}={CellCounter+1, CellCounter+2};
        
        for n=1:2
            CellCounter=CellCounter+1;    
            TriangleMesh.Cell(CellCounter)=tempCell{n};
        end
            
    elseif L_k == 4
        
         CellCounter=CellCounter+1;    
         TriangleMesh.Cell(CellCounter)=Cell_k;        
    else
        disp('invalid polymesh')
        return
    end
end

