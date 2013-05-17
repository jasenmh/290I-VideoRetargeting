% SeamCarving.m
% A function to remove the lowest-energy lines vertically and/or
% horisontally from an image.
% ImgName: a string containing the image file name
% hor: the number of horizontal lines to carve from the image
% ver: the number of vertical lines to carve from the image
% resizedImage: a matrix containing the resized image after seam carving
function [resizedImage] = SeamCarving(ImgName, hor, ver)
inputImage = imread(ImgName);
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
imshow(inputImage);
figure
imshow(ReducedImage);
end

function [ReducedImage,ReducedGradMean] = RemoveVer(ReducedGradMean, ReducedImage)
EnergyMapVer = CalcEnergyMap(ReducedGradMean);
SeamVer = findSeam(EnergyMapVer);
ReducedImage = RemoveSeam(ReducedImage,SeamVer);
ReducedGradMean = RemoveSeam(ReducedGradMean, SeamVer);
end
function [ReducedImage,ReducedGradMean] = RemoveHor(ReducedGradMean, ReducedImage)
EnergyMapHor = CalcEnergyMap(ReducedGradMean');
SeamHor = findSeam(EnergyMapHor);
ReducedImage = RemoveSeam(permute(ReducedImage, [2,1,3]), SeamHor);
ReducedGradMean = RemoveSeam(ReducedGradMean, SeamHor);
ReducedImage = permute(ReducedImage, [2,1,3]);
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
if(dim~=1)
ReducedImage = uint8(ReducedImage);
end
%input = inputImage(:,:,1);
end