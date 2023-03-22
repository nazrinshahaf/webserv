function deleteSomething() {
    toDelete = document.getElementById("deleteText");
    fetch(toDelete.value,  {
      method: 'DELETE'
    })
  }