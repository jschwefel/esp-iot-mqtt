function testResults () {
    document.getElementById('passwd').setAttribute('value', "Jason");
}

function togglePasswordVisibility() {
    var passwordField = document.getElementById("wifi-pass");
    if (passwordField.type === "password") {
        passwordField.type = "text";
    } else {
        passwordField.type = "password";
    }
}