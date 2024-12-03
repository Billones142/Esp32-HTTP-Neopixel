for i = 0, 255 do
    setPixelColor(i, colorWheel(i))
    showPixels()
    delay(5)
end