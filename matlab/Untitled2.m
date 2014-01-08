str = '**HWCOLOR COMP          1    53';

C = textscan(str,'%s');
%%
FullFileName='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open.inp';

tic
PolyMesh = ReadPolyMeshDataFile_inp(FullFileName);
toc
%%
TriangleMesh=TriangleMeshClass;
tic
TriangleMesh.InitializeFromPolyMesh(PolyMesh);
toc
%%
tic
[FigureHandle, Result1] = vtkfigure();
toc
%%
tic
[PropHandle, Result] = vtkshowpolymesh(FigureHandle, PolyMesh);
toc
%%
tic
[Handle, Result] = vtkshowtrianglemesh(FigureHandle, TriangleMesh);
toc