// Dynamically loads list contents.
function loadListContent(id, start, count)
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
  {
    if ((request.readyState == 4) && (request.status == 200) &&
        (request.responseText.length > 0))
    {
      document.getElementById(id).innerHTML += request.responseText;
      loadListContent(id, start + count, count);
    }
  }

  request.open("GET", "?" + id + "=" + start + "," + count, true);
  request.send();
}
