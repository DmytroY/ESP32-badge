const colorMap = {
    "0x0000": [ // Black Background
        { name: "White",  value: "0xffff" },
        { name: "Green",  value: "0x37e6" },
        { name: "Blue",   value: "0xfc30" },
        { name: "Pink",    value: "0x843f" },
        { name: "Red",    value: "0x295f" },
        { name: "Yellow", value: "0x87ff" },
        { name: "Cyan",   value: "0xfff0" }
    ],
    "0xFFFF": [ // White Background
        { name: "Black",   value: "0x0000" },
        { name: "Dark Blue",   value: "0x6000" },
        { name: "Dark Green",  value: "0x0300" },
        { name: "Dark Red",    value: "0x0015"},
        { name: "Dark Violet", value: "0x4008" }
    ]
};

function updateTextColors() {
    const bgColor = document.getElementById('bgColor').value;
    const textColorSelect = document.getElementById('textColor');
    
    // Clear existing options
    textColorSelect.innerHTML = "";

    // Add new filtered options
    colorMap[bgColor].forEach(color => {
        const opt = document.createElement('option');
        opt.value = color.value;
        opt.innerText = color.name;
        textColorSelect.appendChild(opt);
    });
}

// Initialize the list on page load
window.onload = updateTextColors;

async function sendData(action) {
    const title = document.getElementById('title').value;
    const subtitle = document.getElementById('subtitle').value;
    const qr = document.getElementById('qr').value;
    const bright = document.getElementById('bright').value;
    const textColor = document.getElementById('textColor').value;
    const bgColor = document.getElementById('bgColor').value;
    const status = document.getElementById('status');

    // 1. Check for obligatory Title
    if (!title && action === 'finish') {
        alert("Title is obligatory");
        return;
    }

    // 2. Check for Color Conflict
    // if (textColor === bgColor) {
    //     status.innerText = "Error: Text and Background cannot be the same color.";
    //     status.style.color = "red";
    //     return;
    // }

    status.style.color = "#888"; // Reset status color

    const params = new URLSearchParams({
        title, subtitle, qr, action,
        bright, textColor, bgColor
    });

    try {
        const response = await fetch(`/msg?${params.toString()}`);
        if (response.ok) {
            status.innerText = action === 'test' ? "Preview updated" : "Finished. ESP shutting down...";
        }
    } catch (err) {
        status.innerText = "Error connecting to ESP32";
    }
}