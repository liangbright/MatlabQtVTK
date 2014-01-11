clear

tic
[FigureHandle, Result0] = vtkfigure();
toc
%%
Point=[0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9];

Color=[1, 0, 0];

tic
Result1 = vtkplotpoint(FigureHandle, Point, Color);
toc
%%
Vector=[0 1 2 3 4 5 6 7 8 9 10 11 12
        0 1 2 3 4 5 6 7 8 9 10 11 12
        0 1 2 3 4 5 6 7 8 9 10 11 12];
      
Position=[0 1 2 3 4 5 6 7 8 9 10 11 12
          0 1 2 3 4 5 6 7 8 9 10 11 12
          0 1 2 3 4 5 6 7 8 9 10 11 12];

VectorData=[Vector
            Position];

Color=[1, 1, 0];

tic
Result2 = vtkplotvector(FigureHandle, VectorData, Color);
toc
%%
Tensor=[0 1 2
        3 4 5
        6 7 8];
    
Position=[-1
          -1
          -1];

TensorData(:,1)=[Tensor(:); Position(:,1)];

Color=[1, 1, 0];

Result2 = vtkplottensor(FigureHandle, TensorData, Color);