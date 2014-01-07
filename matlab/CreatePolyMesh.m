function Mesh=CreatePolyMesh() 

InputNum=nargin;

if InputNum == 0

    Mesh.Type='Poly';

    Mesh.PointNum=0;
    Mesh.Point=[];

    Mesh.PointIDList=[];

    Mesh.CellNum=0;
    Mesh.Cell={};
    
    Mesh.PartNum=0;
    Mesh.Part=[];
end