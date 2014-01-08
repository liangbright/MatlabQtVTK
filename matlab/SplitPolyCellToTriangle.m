function Triangle=SplitPolyCellToTriangle(PolyCell)

Triangle={};

PointNum=length(PolyCell);

if PointNum == 4
    
    Triangle=cell(1,2);
    
    % 1,2,3 -> cell_k_1
     % 3,4,1 -> cell_k_2
    
    Triangle{1}={PolyCell(1), PolyCell(2), PolyCell(3)};
    Triangle{2}={PolyCell(3), PolyCell(4), PolyCell(1)};
else
    disp('to do @ SplitPolyCellToTriangle')
end