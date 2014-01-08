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
[Handle1, Result1] = vtkshowpolymesh(FigureHandle1, PolyMesh{1}, 'green');
[Handle2, Result2] = vtkshowpolymesh(FigureHandle1, PolyMesh{2}, 'yellow');
%%
FigureHandle2 = vtkfigure();
[Handle3, Result3] = vtkshowpolymesh(FigureHandle2, PolyMesh{3}, 'green');
[Handle4, Result4] = vtkshowpolymesh(FigureHandle2, PolyMesh{4}, 'red');
%%
FigureHandle3 = vtkfigure();
[Handle5, Result5] = vtkshowpolymesh(FigureHandle3, PolyMesh{5}, 'green');
[Handle6, Result6] = vtkshowpolymesh(FigureHandle3, PolyMesh{6}, 'red');
%%
FullPath='E:\HeartData\P1943091-im_6-phase10-close-leaflet\im_6\phase0\';
Volume = ReadDIOMVolume(FullPath);
%%
FigureHandle4 = vtkfigure();
Result = vtkshowvolume(FigureHandle4, Volume);