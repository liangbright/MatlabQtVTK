clear

tic
FigureHandle = vtkfigure();
toc
%%
Point=[0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9]*8;

Color=[1, 1, 1];

if ~isempty(FigureHandle)
    tic
    [PropHandle, Result] = vtkplotpoint(FigureHandle, Point, Color);
    toc
end