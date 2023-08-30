
function togglePasswordVisibility(elementid) {
    var passwordField = document.getElementById(elementid);
    if (passwordField.type === "password") {
        passwordField.type = "text";
    } else {
        passwordField.type = "password";
    }
}
function updateHiddenInput(elementid) {
    var dropdown = document.getElementById('select-' + elementid);
    var selectedOption = dropdown.options[dropdown.selectedIndex];

    var hiddenInput = document.getElementById(elementid);
    hiddenInput.value = selectedOption.value; // Store the actual submitted value
}

function incrementNumber(elementid) {
    var input = document.getElementById(elementid);
    input.stepUp();
}

function decrementNumber(elementid) {
    var input = document.getElementById(elementid);
    input.stepDown();
}