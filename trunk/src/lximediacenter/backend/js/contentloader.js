// Recursively loads list contents.
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

// Loads a player based on the window size.
function loadPlayer(id, playerUrl)
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
  {
    if ((request.readyState == 4) && (request.status == 200) &&
        (request.responseText.length > 0))
    {
      document.getElementById(id).innerHTML = request.responseText;
    }
  }

  var width = 630, height = 460;
  if (window.innerWidth && window.innerHeight) 
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }
  
  height -= 40;

  request.open("GET", playerUrl + "&size=" + width + "x" + height, true);
  request.send();
}
