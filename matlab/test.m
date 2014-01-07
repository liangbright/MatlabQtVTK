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
Volume=[];
Volume(:,:,1)=[1, 1, 1
               1, 1, 1];
           
Volume(:,:,2)=[2, 2, 2
               2, 2, 2];

%[Ly, Lx, Lz]=size(Volume);

%%
Volume=[];
for k=1:10
Volume(:,:,k)=ones(100,50); 
end

for k=11:20
    Volume(:,:,k)=10*ones(100,50);
end
%%
Volume=[];
Volume(:,:,1)=[1, 2, 3
               4, 5, 6];
           
Volume(:,:,2)=[7,   8, 9
               10, 11, 12];
[Ly, Lx, Lz]=size(Volume);
for z=1:Lz
    for y=1:Ly
        for x=1:Lx
            disp(num2str(Volume(y,x,z)))
        end
    end
end
%%

[PropHandle_volume, Result3] = vtkshowvolume(FigureHandle, Volume);
%%
Mesh.Point={{1, 0, 0, 0}, ...
            {2, 1, 0, 0}, ...
            {3, 0, 1, 0}, ...
            {4, 1, 1, 0},...
            {5, 0, 0, 1},...
            {6, 1, 0, 1},...
            {7, 0, 1, 1},...
            {8, 1, 1, 1}};

Mesh.Cell={{1, 1, 2, 3}, ...
           {2, 1, 2, 5},...
           {2, 1, 5, 3}};

[PropHandle, Result] = vtkshowpolymesh(FigureHandle, Mesh);