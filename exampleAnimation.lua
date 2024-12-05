for i= 0, 255 do
    for index= 0, getPixelAmount()-1 do
        setPixelColor(index,colorWheel(i))
    end
    showPixels()
    delay(20)
end