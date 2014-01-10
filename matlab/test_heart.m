%%
FullFileName{1}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open.inp';
FullFileName{2}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open-leaflet.inp';
FullFileName{3}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase2-halfopen.inp';
FullFileName{4}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase2-halfopen-leaflet.inp';
FullFileName{5}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase10-close.inp';
FullFileName{6}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase10-close-leaflet.inp';
%%
PolyMesh=cell(1,6);
for k=1:6
tic
PolyMesh{k} = ReadPolyMeshDataFile_inp(FullFileName{k});
toc
end
%%
FigureHandle1 = vtkfigure();
Result1 = vtkshowpolymesh(FigureHandle1, PolyMesh{1}, 'green');
Result2 = vtkshowpolymesh(FigureHandle1, PolyMesh{2}, 'yellow');
%%
FigureHandle2 = vtkfigure();
Result3 = vtkshowpolymesh(FigureHandle2, PolyMesh{3}, 'green');
Result4 = vtkshowpolymesh(FigureHandle2, PolyMesh{4}, 'red');
%%
FigureHandle3 = vtkfigure();
Result5 = vtkshowpolymesh(FigureHandle3, PolyMesh{5}, 'green');
Result6 = vtkshowpolymesh(FigureHandle3, PolyMesh{6}, 'red');
%%
FilePath='E:\HeartData\P1943091-im_6-phase10-close-leaflet\im_6\phase0\';
Volume = ReadDIOMVolume(FilePath);
%%
VolumeFigureHandle = vtkfigure();
%
IntensityDisplayRange=[0, 2000];
Result7 = vtkshowvolume(VolumeFigureHandle, Volume, IntensityDisplayRange);
VolumeHandle=Result7.PropHandle;
%%
% Volume(y+1,x+1,z+1) is the voxel at (x, y, z)
SlicePlane.Normal=[0, 0, 1];  % (x,y,z)
SlicePlane.Origin=[0, 0, 9]; % (x,y,z)
Result8 = vtkshowsliceofvolume(VolumeFigureHandle,VolumeHandle, SlicePlane, IntensityDisplayRange);
%%
tempResult=vtkshowaxes(VolumeFigureHandle);
%%
imtool(Volume(:,:,10))
%% ================================= test TriangleMesh ===============================================================
TriangleMesh=cell(1,6);
for k=1:6
tic
TriangleMesh{k}=TriangleMeshClass;
TriangleMesh{k}.InitializeFromPolyMesh(PolyMesh{k});
toc
end
%%
FigureHandle1 = vtkfigure();
Result1 = vtkshowtrianglemesh(FigureHandle1, TriangleMesh{1}, 'green');
Result2 = vtkshowtrianglemesh(FigureHandle1, TriangleMesh{2}, 'yellow');
tempResult=vtkshowaxes(FigureHandle1);
%%
FigureHandle2 = vtkfigure();
Result3 = vtkshowtrianglemesh(FigureHandle2, TriangleMesh{3}, 'green');
Result4 = vtkshowtrianglemesh(FigureHandle2, TriangleMesh{4}, 'red');
%%
FigureHandle3 = vtkfigure();
Result5 = vtkshowtrianglemesh(FigureHandle3, TriangleMesh{5}, 'green');
Result6 = vtkshowtrianglemesh(FigureHandle3, TriangleMesh{6}, 'red');