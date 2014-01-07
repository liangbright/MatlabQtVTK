function Mesh=CreateTriangleMesh() 

InputNum=nargin;

if InputNum == 0

    Mesh.Type='Triangle';

    Mesh.PointNum=0;
    Mesh.Point=[];

    Mesh.PointIDList=[];

    Mesh.CellNum=0;
    Mesh.Cell={};
    
    Mesh.PartNum=0;
    Mesh.Part=[];
end