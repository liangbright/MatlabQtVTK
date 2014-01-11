clear

tic
[FigureHandle, Result1] = vtkfigure();
toc
%%
Point=[0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9 
       0 1 2 3 4 5 6 7 8 9];

Color=[1, 0, 0];

tic
Result2 = vtkplotpoint(FigureHandle, 'PointAAA', Point,  Color);
toc
%%
[handle, Result] = vtkremoveprop(Result2.FigureHandle, Result2.PropHandle);
%%
Line=[0 1 2 3 4 5 6 7 8 9 10 11 12
      0 1 2 3 4 5 6 7 8 9 10 11 12
      0 1 2 3 4 5 6 7 8 9 10 11 12];

Color=[1, 1, 0];

tic
Result2 = vtkplotline(FigureHandle, 'LineBBB', Line,  Color);
toc
%%
theta=0:pi/12:2*pi;
Line=[];
Line(1,:)=cos(theta);
Line(2,:)=sin(theta);
Line(3,:)=1;

Color=[1, 1, 0];

Result2 = vtkplotline(FigureHandle, 'LineCCC', Line,  Color);