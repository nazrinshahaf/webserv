function deleteSomething() {
    toDelete = document.getElementById("deleteText");
    fetch("/delete/" + toDelete.value,  {
      method: 'DELETE',
    })
    .then((data) => {
        console.log(data);
        window.alert(data.status + " " + data.statusText);
    });
}