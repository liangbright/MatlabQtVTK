str = '**HWCOLOR COMP          1    53';

C = textscan(str,'%s');
%%
FullFileName='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open.inp';

tic
PolyMesh = ReadPolyMeshDataFile_inp(FullFileName);
toc
%%
tic
IsOK1 = PolyMesh.CheckConsistency();
toc
%%
tic
PolyMesh.ChangeIndexFromGlobalToLocal();
toc
%%
TriangleMesh=TriangleMeshClass;

TriangleMesh.InitializeFromPolyMesh(PolyMesh);
%%
tic
[FigureHandle, Result1] = vtkfigure();
toc
%%
[PropHandle, Result] = vtkshowpolymesh(FigureHandle, PolyMesh);