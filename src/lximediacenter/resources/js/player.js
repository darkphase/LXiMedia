// Show and hide controls.
var controlsLocked, timer;

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
var width, height, lastImageFileUrl;

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
  
  var browser = document.getElementById("browser");
  if (browser)
  {
    browser.style.width = width + "px";
    browser.style.height = (height - document.getElementById("audioplayer").offsetHeight) + "px";
  }
  
  if (lastImageFileUrl)
    loadImage(lastImageFileUrl);
}

function closePlayer()
{
  top.location.href = document.getElementById("browser").contentWindow.location.href;
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

  request.open("GET", path + "?items=" + start + "," + count + "&func=2,loadVideo,3,loadImage", true);
  request.send();
}

function loadAudio(audioFileUrl)
{
  lastImageFileUrl = false;
  
  if (top.location.href != window.location.href)
  {
    top.location.href = window.location.href;
  }
  else
  {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function()
      {
        if ((request.readyState == 4) && (request.status == 200) &&
            (request.responseText.length > 0))
        {
          document.getElementById("player").innerHTML = request.responseText;

          resizeWindow();
        }
      }

    request.open("GET", audioFileUrl + "?audioplayer=", true);
    request.send();
  }
}

function loadVideo(videoFileUrl)
{
  lastImageFileUrl = false;
  
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
    {
      if ((request.readyState == 4) && (request.status == 200) &&
          (request.responseText.length > 0))
      {
        document.getElementById("player").innerHTML = request.responseText;
      }
    }

  request.open("GET", videoFileUrl + "?videoplayer=", true);
  request.send();
}

function loadImage(imageFileUrl)
{
  if (!lastImageFileUrl)
    resizeWindow();

  lastImageFileUrl = imageFileUrl;

  document.getElementById("player").innerHTML = 
      "<img src=\"" + imageFileUrl + "?format=jpeg&resolution=" + width + "x" + height + "&bgcolor=000000\" alt=\"...\" />";
}
