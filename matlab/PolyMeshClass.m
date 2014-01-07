classdef PolyMeshClass < handle
    
properties (GetAccess=public, SetAccess=private)

Point=[];
% row 1: Point_idx in Cell
% row 2: x
% row 3: y
% row 4: z

PointGlobalIdxList=[]; %Global Index of each point, may not start from 1

Cell={};
% {Cell_1, Cell_2}
% Cell_k={cell_idx, point1_idx, point2_idx, point3_idx, ...}

IndexTypeInCell=''; % Point/cell Index type (point#_idx) in Cell, 

CellGlobalIdxList=[]; %Global Index of each point, may not start from 1

Part_PointIdxTable=[];
% row 1: start point_idx of part 1,  start point_idx of part 2, ...
% row 2: end point_idx of part 1,    end point_idx of part 2,   ...

Part_CellIdxTable=[];
% row 1: start cell_idx point of part 1,  start cell_idx of part 2, ...
% row 2: end cell_idx of part 1,          end cell_idx of part 2,   ...

end
    
methods
        
function this=PolyMeshClass()

end


function SetMesh(this, Mesh)

%{
Mesh.Point(:,1)=[1, 0, 0, 0];
Mesh.Point(:,2)=[2, 1, 0, 0];
Mesh.Point(:,3)=[3, 0, 1, 0];
Mesh.Point(:,4)=[4, 1, 1, 0];
Mesh.Point(:,5)=[5, 0, 0, 1];
Mesh.Point(:,6)=[6, 1, 0, 1];
Mesh.Point(:,7)=[7, 0, 1, 1];
Mesh.Point(:,8)=[8, 1, 1, 1];

Mesh.Cell={[1, 1, 2, 3], ...
           [2, 1, 2, 5],...
           [2, 1, 5, 3]};
%}       
this.Point=Mesh.Point;
[~, PointNum]=size(Mesh.Point);
if isfield(Mesh, 'PointGlobalIdxList')
    this.PointGlobalIdxList=Mesh.PointGlobalIdxList;
else
    this.PointGlobalIdxList=1:PointNum;
end

this.Cell=Mesh.Cell;
CellNum=length(Mesh.Cell);
if isfield(Mesh, 'CellGlobalIdxList')
    this.CellGlobalIdxList=Mesh.CellGlobalIdxList;
else
    this.CellGlobalIdxList=1:CellNum;
end

if isfield(Mesh, 'IndexTypeInCell')
    this.IndexTypeInCell=Mesh.IndexTypeInCell;
else
    this.IndexTypeInCell='Local';
end
end

%===============================================================================================================
function IsOK = CheckConsistency(this)

IsOK=1;

CellNum=length(this.Cell);

tempPointIDList=this.PointGlobalIdxList;
tempCell=this.Cell;

for k=1:CellNum
    
    Cell_k=tempCell{k};
    
    Num_k=length(Cell_k);
    if Num_k == 0
        disp(['Error in Mesh.Cell{k}, k=' num2str(k)]);
        IsOK=0;    
    end
    
    CellIdx_k=Cell_k(1);        
    for n=2:length(Cell_k)-1
        PointIdx=Cell_k(n);
        if sum(tempPointIDList==PointIdx) < 1
            disp(['The index: ' num2str(PointIdx)  ' is not used in Cell: ' num2str(CellIdx_k)])
            IsOK=0;
        end
    end
end

end

%===============================================================================================================
function ChangeIndexFromGlobalToLocal(this)

if ~strcmp(this.IndexTypeInCell, 'Global')
    disp('wrong in ChangeIndexFromGlobalToLocal')
    return
end

CellNum=length(this.Cell);

for k=1:CellNum

    Cell_k=this.Cell{k};
    
    L_k=length(Cell_k);
    
    Cell_k(1)=k;
    
    for n=2:L_k
        
        LocalIdx=find(this.PointGlobalIdxList==Cell_k(n));

        Cell_k(n)=LocalIdx;
    end

end

this.IndexTypeInCell='Local';

end


function ChangeIndexFromLocalToGlobal(this)

if ~strcmp(this.IndexTypeInCell, 'Local')
    disp('wrong in ChangeIndexFromLocalToGlobal')
    return
end

CellNum=length(this.Cell);

for k=1:CellNum

    Cell_k=this.Cell{k};
    L_k=length(Cell_k);
    Cell_k(1)=this.CellGlobalIdxList(k);
    for n=2:L_k
        Cell_k(n)=this.PointGlobalIdxList(Cell_k(n));
    end
end    

this.IndexTypeInCell='Global';

end
%===============================================================================================================
end
end