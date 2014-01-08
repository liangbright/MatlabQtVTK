classdef PolyMeshClass < handle
    
properties (GetAccess=public, SetAccess=private)

Point=[];
% row 1: x
% row 2: y
% row 3: z

PointGlobalIdxList=[]; %Global Index of each point, may not start from 1

Cell={};
% {Cell_1, Cell_2}
% Cell_k={point1_idx, point2_idx, point3_idx, ...}

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
Mesh.Point(:,1)=[0, 0, 0];
Mesh.Point(:,2)=[1, 0, 0];
Mesh.Point(:,3)=[0, 1, 0];
Mesh.Point(:,4)=[1, 1, 0];
Mesh.Point(:,5)=[0, 0, 1];
Mesh.Point(:,6)=[1, 0, 1];
Mesh.Point(:,7)=[0, 1, 1];
Mesh.Point(:,8)=[1, 1, 1];

Mesh.Cell={[1, 2, 3], ...
           [1, 2, 5],...
           [1, 5, 3]};
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

end

function Mesh=GetMesh(this)
Mesh.Point=this.Point;
Mesh.PointGlobalIdxList=this.PointGlobalIdxList;
Mesh.Cell=this.Cell;
Mesh.CellGlobalIdxList=this.CellGlobalIdxList;
end
%===============================================================================================================
function IsOK = CheckConsistency(this)
IsOK=1;
end

%===============================================================================================================
end
end