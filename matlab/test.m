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
Volume=100*rand(100,100,2);
Volume(:,:,1)=100*ones(100,100);
[PropHandle_volume, Result3] = vtkshowvolume(FigureHandle, Volume);
%%
Mesh.Point={{1, 0, 0, 0}, ...
            {2, 1, 0, 0}, ...
            {3, 0, 1, 0}, ...
            {4, 1, 1, 0},...
            {5, 0, 0, 1},...
            {6, 1, 0, 1}};

Mesh.Cell={{1, 1, 2, 3}, ...
           {2, 4, 5, 6}};

[PropHandle, Result] = vtkshowpolymesh(FigureHandle, Mesh);