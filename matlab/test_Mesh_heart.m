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
[FigureHandle, Result1] = vtkfigure();
%%
tic
[Handle, Result] = vtkshowpolymesh(FigureHandle, PolyMesh{5});
toc
