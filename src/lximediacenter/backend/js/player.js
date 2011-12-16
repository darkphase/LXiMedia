// Show and hide controls.
var controlsLocked;

function showControls()
{
  if (!controlsLocked)
  {
    clearTimeout(timer);
    timer = setTimeout("hideControls()", 1000);
  
    document.getElementById("controls").style.visibility = "visible";
  }
}

function hideControls()
{
  if (!controlsLocked)
  {
    document.getElementById("controls").style.visibility = "hidden";
  }
}

function lockControls()
{
  clearTimeout(timer);
  controlsLocked = true;
  
  document.getElementById("controls").style.visibility = "visible";
}

function unlockControls()
{
  timer = setTimeout("hideControls()", 1000);
  controlsLocked = false;
}

// Player
var width, height, timer, lastImageUrl;

function resizeWindow()
{
  width = 630;
  height = 460;
  
  if (window.innerWidth && window.innerHeight) 
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }
  else if (document.body && document.body.offsetWidth) 
  {
    width = document.body.offsetWidth;
    height = document.body.offsetHeight;
  }
  
  if (lastImageUrl)
    loadImage(lastImageUrl);
}

function loadImage(imageUrl)
{
  if (!lastImageUrl)
    resizeWindow();

  lastImageUrl = imageUrl;

  document.getElementById("player").innerHTML = 
      "<img src=\"" + imageUrl + "?format=jpeg&resolution=" + width + "x" + height + "&bgcolor=000000\" alt=\"...\" />";
}

function loadThumbnailBar(path, start, count)
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
    {
      if ((request.readyState == 4) && (request.status == 200) &&
          (request.responseText.length > 0))
      {
        document.getElementById("items").innerHTML += request.responseText;
        if (count > 0)
          loadThumbnailBar(path, start + count, count);
      }
    }

  request.open("GET", path + "?items=" + start + "," + count + "&type=30" + "&func=loadImage", true);
  request.send();
}
