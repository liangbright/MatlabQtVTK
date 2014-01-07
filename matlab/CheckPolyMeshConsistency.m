function IsOK = CheckPolyMeshConsistency(Mesh)

IsOK=1;

CellNum=length(Mesh.Cell);

for k=1:CellNum
    
    Cell_k=Mesh.Cell{k};
    
    Num_k=length(Cell_k);
    if Num_k == 0
        disp(['Error in Mesh.Cell{k}, k=' num2str(k)]);
        IsOK=0;    
    end
    
    CellIdx_k=Cell_k(1);        
    for n=2:length(Cell_k)-1
        PointIdx=Cell_k(n);
        if sum(Mesh.PointIdxList==PointIdx) <1
            disp(['The index: ' num2str(PointIdx)  ' is not used in Cell: ' num2str(CellIdx_k)])
            return
        end
    end
end