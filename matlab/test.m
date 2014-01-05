clear

tic
[FigureHandle, Result1] = vtkfigure();
toc
%%
Point=[0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9];

Color=[1, 1, 1];

if ~isempty(FigureHandle)
    tic
    [PropHandle_point, Result2] = vtkplotpoint(FigureHandle, Point, Color);
    toc
end
%%
Volume=100*rand(100,100,10);

[PropHandle_volume, Result3] = vtkshowvolume(FigureHandle, Volume);
