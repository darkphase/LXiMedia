// Recursively loads list contents.
function loadListContent(id, path, start, count)
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
  {
    if ((request.readyState == 4) && (request.status == 200) &&
        (request.responseText.length > 0))
    {
      document.getElementById(id).innerHTML += request.responseText;
      if (count > 0)
        loadListContent(id, path, start + count, count);
    }
  }

  request.open("GET", path + "?" + id + "=" + start + "," + count, true);
  request.send();
}
