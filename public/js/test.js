let button = document.getElementById("button");
if (button) {
  button.addEventListener("click", test);
}

function test() {
  let header = document.querySelectorAll("h1")[0];

  if (header) {
    header.innerText = "AHAHAH AHAHAH AHAHAH";
  }
}
