function version()              { return "0.3.0"; }
function targetAudience()       { return "NL"; }

function listItems(path)
{
  if (path == "/")
  {
    var items =
      [ // Radio
        // TV
        ["rtlz",        "RTL Z",        "Video"] 
      ];

    return items;
  }
  else
    return [];
}

function icon(id)
{ 
  return "";
}

function streamLocation(id)
{
  if      (id == "rtlz")        return "http://rtlztv.rtl.nl/rtlzbroad?MSWMExt=.asf";
  else                          return "";
}
