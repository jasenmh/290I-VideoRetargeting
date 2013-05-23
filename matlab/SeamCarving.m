function [resizedImage] = SeamCarving(ImgName, hor, ver)
inputImage = imread(ImgName);
inputImage = double(inputImage)/255;
figure;
%imshow(inputImage);
GradMean = CalcGradient(inputImage);
%EnergyMapVer = CalcEnergyMap(GradMean);
%EnergyMapHor = CalcEnergyMap(GradMean');
%EnergyMapHor = EnergyMapHor';
ReducedImage = inputImage;
%ReducedEnergyMapVer = EnergyMapVer;
%ReducedEnergyMapHor = EnergyMapHor;
ReducedGradMean = GradMean;
for i=1:1:min(ver, hor)
[ReducedImage,ReducedGradMean] = RemoveVer(ReducedGradMean, ReducedImage);
[ReducedImage,ReducedGradMean] = RemoveHor(ReducedGradMean, ReducedImage);
end
for j=1:1:(max(ver,hor)-min(ver,hor))
    if(ver>hor)
        [ReducedImage,ReducedGradMean] = RemoveVer(ReducedGradMean, ReducedImage);
    else
        [ReducedImage,ReducedGradMean] = RemoveHor(ReducedGradMean, ReducedImage);
    end
end
% figure
% imshow(ReducedImage);
% ReducedImage = uint8(ReducedImage*255);
imshow(inputImage);
figure
imshow(ReducedImage);
end

function [ReducedImage,ReducedGradMean] = RemoveVer(ReducedGradMean, ReducedImage)
EnergyMapVer = CalcEnergyMap(ReducedGradMean);
SeamVer = findSeam(EnergyMapVer);
ReducedImage = RemoveSeam(ReducedImage,SeamVer);
ReducedGradMean = RemoveSeamGradient(ReducedGradMean, SeamVer);
end
function [ReducedImage,ReducedGradMean] = RemoveHor(ReducedGradMean, ReducedImage)
EnergyMapHor = CalcEnergyMap(ReducedGradMean');
SeamHor = findSeam(EnergyMapHor);
ReducedImage = RemoveSeam(permute(ReducedImage, [2,1,3]), SeamHor);
ReducedGradMean = RemoveSeamGradient(permute(ReducedGradMean,[2,1,3]), SeamHor);
ReducedImage = permute(ReducedImage, [2,1,3]);
ReducedGradMean = permute(ReducedGradMean, [2,1,3]);
end
% calculating Energy Map for input image
function [GradMean] = CalcGradient(inputImage)
    sobelOpx = [ 1 0 -1 ; 2  0  -2; 1 0 -1];
    sobelOpy = [1 2 1; 0 0 0; -1 -2 -1];
    [rows, cols, dim] = size(inputImage);
    for i= 1:1:dim
        GradHor(:,:,i) = filter2(sobelOpx, inputImage(:,:,i));
        GradVer(:,:,i) = filter2(sobelOpy, inputImage(:,:,i));
        Grad(:,:,i) = abs(GradHor(:,:,i)) + abs(GradVer(:,:,i));
    end
    GradMean = sum(Grad, 3)/dim;
end
function [EnergyMap] = CalcEnergyMap(GradMean)
    [rows, cols] = size(GradMean);
    for i = 1:1:rows
        for j = 1:1:cols
            if(i== 1)
                EnergyMap(1,:) = GradMean(1,:);
            else
                if(j == 1)
                    EnergyMap(i,j) = GradMean(i,j)+ min(EnergyMap(i-1,j), EnergyMap(i-1,j+1));
                elseif(j == cols)
                    EnergyMap(i,j) = GradMean(i,j)+ min(EnergyMap(i-1,j-1), EnergyMap(i-1,j));
                else
                    EnergyMap(i,j) = GradMean(i,j)+ min(min(EnergyMap(i-1,j-1), EnergyMap(i-1,j)), EnergyMap(i-1,j+1));
                end
                    
            end
        end
    end
end

function[Seam] = findSeam(EnergyMap)
[rows, cols] = size(EnergyMap);
Seam = zeros(rows,1);
for k = 1:1:rows
    if(k==1)
        [Value, Seam(rows)] = min(EnergyMap(rows,:)); 
    else
        if(Seam(rows-k+2) == 1)
             [Value, temp] = min(EnergyMap(rows-k+1,1:2));
             Seam(rows-k+1)= temp;
        elseif(Seam(rows-k+2) == cols)
             [Value, temp] = min(EnergyMap(rows-k+1,cols-1:cols));
             Seam(rows-k+1)= Seam(rows-k+2)-2 + temp;
        else
        [Value, temp] = min(EnergyMap(rows-k+1,Seam(rows-k+2)-1:1:Seam(rows-k+2)+1));
        Seam(rows-k+1)= Seam(rows-k+2)-2 + temp;
        end
    end
end
end
function [ReducedImage] = RemoveSeam(inputImage, Seam)
[rows, cols, dim] = size(inputImage);
ReducedImage = zeros(rows,cols-1, dim);
%imshow(inputImage);
for k=1:1:dim
    for i=1:1:rows
        for j=1:1:cols-1
            if(j < Seam(i))
                ReducedImage(i,j,k) = inputImage(i,j,k);
            elseif(j >= Seam(i))
                ReducedImage(i,j,k) = inputImage(i,j+1,k);
            end                
        end
    end
        
end
% if(dim~=1)
% ReducedImage = uint8(ReducedImage*255);
% end
%input = inputImage(:,:,1);
end
function [ReducedGradMean] = RemoveSeamGradient(GradMean, Seam)
[rows, cols, dim] = size(GradMean);
ReducedGradMean = zeros(rows,cols-1, dim);
for i=1:1:rows
    for j=1:1:cols-1
%         if(j == Seam(i)-1)
%             GradMean(i,j) = GradMean(i,j) + GradMean(i,Seam(i))/2;
%         end
%         if(j == Seam(i))
%             GradMean(i,j+1) = GradMean(i,j+1) + GradMean(i,Seam(i))/2;
%         end
        if(j < Seam(i))
            ReducedGradMean(i,j) = GradMean(i,j);
        elseif(j >= Seam(i))
            ReducedGradMean(i,j) = GradMean(i,j+1);
        end                
    end
end

end
