for i= 0, 255 do
    if ((i % 2) == 0) then
        for index= 0, getPixelAmount()-1 do
            setPixelColor(index,colorWheel(i))
        end
    else
        clearPixels();
    end
    showPixels()
    delay(20)
end