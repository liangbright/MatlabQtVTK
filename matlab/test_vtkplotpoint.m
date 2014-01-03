FigureHandle = vtkfigure();
%%
PointData=[0 1 2 3 4 5 6 7 8 9 
           0 1 2 3 4 5 6 7 8 9 
           0 1 2 3 4 5 6 7 8 9];

Color=[1, 1, 1];

if ~isempty(FigureHandle)
    PropHandle = vtkplotpoint(FigureHandle, Point, Color);
end