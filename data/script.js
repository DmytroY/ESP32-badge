function controlLED(state) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("status").innerHTML = "Status: " + state.toUpperCase();
            document.getElementById("status").style.color = (state === 'on') ? "#27ae60" : "#c0392b";
        }
    };
    xhttp.open("GET", "/" + state, true);
    xhttp.send();
}