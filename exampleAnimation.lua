function loop()
    for i = 0, 255 do
        setPixelColor(i, colorWheel(i))  -- Azul
        showPixels()
        delay(5)  -- Espera 100 ms
    end
end