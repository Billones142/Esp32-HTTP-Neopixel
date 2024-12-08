const luaScript = document.getElementById('luaScript');

function saveLocalScript() {
    localStorage.setItem("savedScript", luaScript.value);
}

function deleteLocalScript() {
    localStorage.removeItem("savedScript");
}

function setScript() {
    luaScript.value = localStorage.getItem("savedScript");
}
setScript();

document.getElementById("sendScriptData").addEventListener("click", () => {
    sendScriptData();
})

document.getElementById("saveLocalScript").addEventListener("click", () => {
    saveLocalScript();
})

document.getElementById("deleteLocalScript").addEventListener("click", () => {
    deleteLocalScript();
})

luaScript.addEventListener('keydown', function (event) {
    if (event.key === 'Tab') {
        event.preventDefault(); // Evita el comportamiento predeterminado de tabulación
        const start = this.selectionStart;
        const end = this.selectionEnd;

        // Inserta 4 espacios en la posición actual del cursor
        const spaces = '    ';
        this.value = this.value.substring(0, start) + spaces + this.value.substring(end);

        // Coloca el cursor justo después de los 4 espacios
        this.selectionStart = this.selectionEnd = start + spaces.length;
    }
});
const colorSelector = document.getElementById('colorValues');
const autoSendCheckbox = document.getElementById('automaticStaticColourSend');

let debounceTimer;
colorSelector.addEventListener('input', () => {
    if (autoSendCheckbox.checked) {
        clearTimeout(debounceTimer); // Clear the previous timer
        debounceTimer = setTimeout(() => {
            sendStaticData();
        }, 7);
    }
});

function updateMessage(message) {
    document.getElementById('response').textContent = message;

}

function updateValue(color, value) {
    document.getElementById(`${color}Value`).textContent = value;
}

function hexToRgb(hex) {
    const red = parseInt(hex.substring(1, 3), 16); // Extraer y convertir rojo
    const green = parseInt(hex.substring(3, 5), 16); // Extraer y convertir verde
    const blue = parseInt(hex.substring(5, 7), 16); // Extraer y convertir azul
    return { red, green, blue };
}

function sendStaticData() {
    const index = document.getElementById('index').value;
    const { red, green, blue } = hexToRgb(colorSelector.value)

    const jsonPayload = {
        colours: [
            {
                i: parseInt(index),
                r: red,
                g: green,
                b: blue
            }
        ]
    };

    document.getElementById('response').textContent = "";
    fetch('http://esp-32.local/setcolor', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(jsonPayload)
    })
        .then(response => response.json())
        .then(data => {
            updateResponseLabel(data.message);
        })
        .catch(error => {
            updateResponseLabel(`Error: ${error}`);
        });
}

function updateResponseLabel(message) {
    let date = new Date();
    let formattedDate = `${date.getDate()}/${date.getMonth() + 1}/${date.getFullYear()} ${date.getHours()}:${date.getMinutes()}:${date.getSeconds()}`;
    document.getElementById('response').textContent = `${formattedDate}: Respuesta del ESP32: ${message}`;
}

function sendScriptData() {
    const saveScript = document.getElementById('saveScript').checked;
    const applyScript = document.getElementById('applyScript').checked;
    const scriptName = document.getElementById('scriptName').value;
    const applySavedScript = document.getElementById('applySavedScript').checked;

    const jsonPayload = {
        "luaScript": String(luaScript.value),
        "applyScript": Boolean(applyScript),
        "saveScript": Boolean(saveScript),
        "applySavedScript": Boolean(applySavedScript),
        "scriptName": String(scriptName),
    };

    document.getElementById('response').textContent = "";
    fetch('http://esp-32.local/setScript', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(jsonPayload)
    })
        .then(response => response.json())
        .then(data => {
            updateResponseLabel(data.message);
        })
        .catch(error => {
            updateResponseLabel(`Error: ${error}`);
        });
}