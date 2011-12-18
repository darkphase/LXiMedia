function resizeWindow()
{
  var width, height;
  
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
  if (browser && width && height)
  {
    browser.style.width = width + "px";
    browser.style.height = (height - document.getElementById("navigator").offsetHeight) + "px";
  }
}
