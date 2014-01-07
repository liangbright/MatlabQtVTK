Data=rand(100,50,10);

[Ly, Lx, Lz]=size(Data);

temp0=Data(:)';
%
temp1=[];
for x=1:Lx;
    for y=1:Ly
        for z=1:Lz                
            temp1(end+1)=Data(y,x,z);
        end
    end
end
%}   

error1=sum(abs(temp0-temp1));

temp2=[];

for x=1:Lx
    temp=Data(:,x,:);
   % temp=reshape(temp, [Lz, Ly]);
    temp=temp(:);
    temp2(end+1:end+length(temp))=temp;
end

error2=sum(abs(temp0-temp2));


temp3=[];

for z=1:Lz
    temp=Data(:,:,z)';
   % temp=reshape(temp, [Lz, Ly]);
    temp=temp(:);
    temp3(end+1:end+length(temp))=temp;
end

error3=sum(abs(temp0-temp3));

%
temp4=[];
for z=1:Lz;
    for y=1:Ly
        for x=1:Lx                
            temp4(end+1)=Data(y,x,z);
        end
    end
end

error4=sum(abs(temp0-temp4));

%
temp5=[];
for z=1:Lz;
    for x=1:Lx
        for y=1:Ly                
            temp5(end+1)=Data(y,x,z);
        end
    end
end

error5=sum(abs(temp0-temp5));
