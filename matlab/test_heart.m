%% 
FullFileName{1}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open.inp';
FullFileName{2}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase0-open-leaflet.inp';
FullFileName{3}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase2-halfopen.inp';
FullFileName{4}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase2-halfopen-leaflet.inp';
FullFileName{5}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase10-close.inp';
FullFileName{6}='E:\HeartData\P1943091-im_6-phase10-close-leaflet\P1943091-im_6-phase10-close-leaflet.inp';
%% load mesh data
PolyMesh=cell(1,6);
for k=1:6
tic
PolyMesh{k} = ReadPolyMeshDataFile_inp(FullFileName{k});
toc
end
%% plot mesh -open
FigureHandle1 = vtkfigure();
Result1 = vtkshowpolymesh(FigureHandle1, 'phase0-open', PolyMesh{1}, 'green');
Result2 = vtkshowpolymesh(FigureHandle1, 'phase0-open-leaflet', PolyMesh{2}, 'yellow');
%% plot mesh -halfopen
FigureHandle2 = vtkfigure();
Result3 = vtkshowpolymesh(FigureHandle2, 'phase2-halfopen', PolyMesh{3}, 'green');
Result4 = vtkshowpolymesh(FigureHandle2, 'phase2-halfopen-leaflet', PolyMesh{4}, 'red');
%% plot mesh -close
FigureHandle3 = vtkfigure();
Result5 = vtkshowpolymesh(FigureHandle3, 'phase10-close', PolyMesh{5}, 'green');
Result6 = vtkshowpolymesh(FigureHandle3, 'phase10-close-leaflet', PolyMesh{6}, 'red');
%% read 3D image - open
FilePath='E:\HeartData\P1943091-im_6-phase10-close-leaflet\im_6\phase0\';
Volume = ReadDIOMVolume(FilePath);
%% show 3D image and mesh  (-open)
VolumeFigureHandle = vtkfigure();

Origin=[-101.2, -111.5, -231.75];
Spacing=[0.488281, 0.488281, 0.625];
IntensityDisplayRange=[1200, 2000];

Result7 = vtkshowvolume(VolumeFigureHandle, 'Volume1', Volume(:,:,end:-1:1), Origin, Spacing, IntensityDisplayRange);
VolumeHandle=Result7.PropHandle;
%%
vtkshowaxes(VolumeFigureHandle);
vtkshowpolymesh(VolumeFigureHandle, 'phase0-open', PolyMesh{1}, 'green');
vtkshowpolymesh(VolumeFigureHandle, 'phase0-open-leaflet', PolyMesh{2}, 'yellow');
%%
% Volume(y+1,x+1,z+1) is the voxel at (x, y, z)
SlicePlane.Normal=[0, 0, 1];  % (x,y,z)
SlicePlane.Origin=[0, 0, 9]; % (x,y,z)
Result8 = vtkshowsliceofvolume(VolumeFigureHandle, VolumeHandle, 'Slice_z9', SlicePlane, IntensityDisplayRange);
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
Result1 = vtkshowtrianglemesh(FigureHandle1, 'T0', TriangleMesh{1}, 'green');
Result2 = vtkshowtrianglemesh(FigureHandle1, 'T0_L', TriangleMesh{2}, 'yellow');
tempResult=vtkshowaxes(FigureHandle1);
%%
FigureHandle2 = vtkfigure();
Result3 = vtkshowtrianglemesh(FigureHandle2, 'T2', TriangleMesh{3}, 'green');
Result4 = vtkshowtrianglemesh(FigureHandle2, 'T2_L', TriangleMesh{4}, 'red');
%%
FigureHandle3 = vtkfigure();
Result5 = vtkshowtrianglemesh(FigureHandle3, 'T10', TriangleMesh{5}, 'green');
Result6 = vtkshowtrianglemesh(FigureHandle3, 'T10_L', TriangleMesh{6}, 'red');