#define HTML_PAGE "\
<!DOCTYPE html>\n\
<html lang=\"en\">\n\
<head>\n\
  <meta charset=\"UTF-8\">\n\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
  <title>ESP32 Neopixel</title>\n\
  <style>\n\
    textarea {\n\
        width: 100%;\n\
        height: 300px;\n\
        font-family: monospace;\n\
        font-size: 14px;\n\
        background-color: #f4f4f4;\n\
        color: #333;\n\
        border: 1px solid #ccc;\n\
        border-radius: 5px;\n\
        padding: 10px;\n\
        resize: vertical;\n\
    }\n\
    button {\n\
        margin-top: 10px;\n\
        padding: 10px 20px;\n\
        font-size: 16px;\n\
        border: none;\n\
        border-radius: 5px;\n\
        background-color: #007BFF;\n\
        color: white;\n\
        cursor: pointer;\n\
    }\n\
    button:hover {\n\
        background-color: #0056b3;\n\
    }\n\
  </style>\n\
</head>\n\
<body>\n\
  <h1>ESP32 Neopixel</h1>\n\
  <form id=\"colorForm\">\n\
    <h2>Static color</h2>\n\
    <div>\n\
      <label for=\"index\">Índice:</label>\n\
      <input type=\"number\" value=\"0\" id=\"index\" name=\"i\" min=\"0\" required>\n\
      <input name=\"Color Picker\" id=\"colorValues\" type=\"color\"/>\n\
    </div>\n\
    <button type=\"button\" onclick=\"sendData()\">Enviar</button>\n\
  </form>\n\
  <form id=\"scriptForm\">\n\
    <h2>Lua script</h2>\n\
    <div>\n\
      <label for=\"script\">Script</label>\n\
      <textarea type=\"text\" id=\"luaScript\" name=\"luaScript\" required></textarea>\n\
    </div>\n\
    <button type=\"button\" onclick=\"sendScriptData()\">Enviar script</button>\n\
  </form>\n\
  <p id=\"response\"></p>\n\
  <script>\n\
    const textarea = document.getElementById('luaScript');\n\
    textarea.addEventListener('keydown', function(event) {\n\
        if (event.key === 'Tab') {\n\
            event.preventDefault();\n\
            const start = this.selectionStart;\n\
            const end = this.selectionEnd;\n\
            const spaces = '    ';\n\
            this.value = this.value.substring(0, start) + spaces + this.value.substring(end);\n\
            this.selectionStart = this.selectionEnd = start + spaces.length;\n\
        }\n\
    });\n\
    const colorSelector = document.getElementById('colorValues');\n\
    let debounceTimer;\n\
    colorSelector.addEventListener('input', () => {\n\
        clearTimeout(debounceTimer);\n\
        debounceTimer = setTimeout(() => {\n\
            sendData();\n\
        }, 7);\n\
    });\n\
    function sendData() {\n\
        const index = document.getElementById('index').value;\n\
        const {red, green, blue} = hexToRgb(colorSelector.value);\n\
        const jsonPayload = { colours: [{ i: parseInt(index), r: red, g: green, b: blue }] };\n\
        fetch('http://esp-32.local/setcolor', {\n\
            method: 'POST',\n\
            headers: {'Content-Type': 'application/json'},\n\
            body: JSON.stringify(jsonPayload)\n\
        })\n\
        .then(response => response.json())\n\
        .then(data => { updateMessage(data.message); })\n\
        .catch(error => { updateMessage(`Error: ${error}`); });\n\
    }\n\
    function sendScriptData() {\n\
        const luaScript = document.getElementById('luaScript').value;\n\
        fetch('http://esp-32.local/setScript', {\n\
            method: 'POST',\n\
            headers: {'Content-Type': 'application/json'},\n\
            body: JSON.stringify({ luaScript })\n\
        })\n\
        .then(response => response.json())\n\
        .then(data => { updateMessage(data.message); })\n\
        .catch(error => { updateMessage(`Error: ${error}`); });\n\
    }\n\
  </script>\n\
</body>\n\
</html>"
