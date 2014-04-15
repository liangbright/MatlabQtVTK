clear

FigureHandle= vtkfigure();
%%
Volume=[];
for k=1:10
Volume(:,:,k)=ones(100,50); 
end

for k=11:20
    Volume(:,:,k)=10*ones(100,50);
end
%%
Result= vtkshowvolume(FigureHandle, 'Volume', Volume);