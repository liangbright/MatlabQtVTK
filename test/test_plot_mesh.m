clear

tic
FigureHandle = vtkfigure();
toc
%%
Mesh=[];
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
           [1, 5, 3], ...
           [2, 1, 4],...
           [2, 3, 4],...
           [2, 4, 5]};
       
Result= vtkshowpolymesh(FigureHandle, 'Mesh', Mesh, 'orange');