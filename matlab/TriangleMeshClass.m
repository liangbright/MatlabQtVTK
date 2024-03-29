classdef TriangleMeshClass < handle
 
properties (GetAccess=public, SetAccess=private)

Point=[];
% row 1: x
% row 2: y
% row 3: z

PointGlobalIdxList=[]; %Global Index of each point, may not start from 1

Triangle=[];
% row 1: point1_idx in Point
% row 2: point2_idx in Point
% row 3: point3_idx in Point

TriangleGlobalIdxList=[]; %Global Index of each point, may not start from 1

Part_PointIdxTable=[];
% row 1: start point_idx of part 1,  start point_idx of part 2, ...
% row 2: end point_idx of part 1,    end point_idx of part 2,   ...

Part_TriangleIdxTable=[];
% row 1: start cell_idx point of part 1,  start cell_idx of part 2, ...
% row 2: end cell_idx of part 1,          end cell_idx of part 2,   ...

end
    
methods
        
function this=TriangleMeshClass()

end

%===============================================================================================================
function SetMesh(this, Mesh)

this.Point=Mesh.Point;
[~, PointNum]=size(Mesh.Point);
if isfield(Mesh, 'PointGlobalIdxList')
    this.PointGlobalIdxList=Mesh.PointGlobalIdxList;
else
    this.PointGlobalIdxList=1:PointNum;
end

this.Triangle=Mesh.Triangle;
TriangleNum=length(Mesh.Triangle);
if isfield(Mesh, 'TriangleGlobalIdxList')
    this.TriangleGlobalIdxList=Mesh.TriangleGlobalIdxList;
else
    this.TriangleGlobalIdxList=1:TriangleNum;
end
end


function Mesh=GetMesh(this)
Mesh.Point=this.Point;
Mesh.PointGlobalIdxList=this.PointGlobalIdxList;
Mesh.Triangle=this.Triangle;
Mesh.TriangleGlobalIdxList=this.TriangleGlobalIdxList;
end

function Clear(this)
    
this.Point=[];
this.PointGlobalIdxList=[];

this.Triangle=[];
this.TriangleGlobalIdxList=[];

end

%===============================================================================================================
function InitializeFromPolyMesh(this, PolyMesh)

PolyMeshCellNum=length(PolyMesh.Cell);

this.Point=PolyMesh.Point;

this.PointGlobalIdxList=PolyMesh.PointGlobalIdxList;

this.Triangle=zeros(3, PolyMeshCellNum*2);

this.TriangleGlobalIdxList=zeros(1, PolyMeshCellNum*2);
TriangleCounter=0;

for k=1:PolyMeshCellNum
    
    Cell_k=PolyMesh.Cell{k};
    
    L_k=length(Cell_k);
    if  L_k > 3 % not a Triangle
    
        tempTriangle=SplitPolyCellToTriangle(Cell_k);
        
        for n=1:length(tempTriangle)
            TriangleCounter=TriangleCounter+1;    
            this.Triangle(:, TriangleCounter)=tempTriangle{n};
            this.TriangleGlobalIdxList(TriangleCounter)=PolyMesh.CellGlobalIdxList(k);
        end
            
    elseif L_k == 3
        
         TriangleCounter=TriangleCounter+1;    
         this.Triangle(:, TriangleCounter)=Cell_k;        
    else
        disp('invalid polymesh @ ConvertFromPolyMesh')
        
        this.Clear();
        
        return
    end
end

this.Triangle=this.Triangle(:,1:TriangleCounter);
this.TriangleGlobalIdxList=this.TriangleGlobalIdxList(:,1:TriangleCounter);
end


%===============================================================================================================
end
end