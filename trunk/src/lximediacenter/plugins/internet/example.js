// Access to internet based media such are web-radio, web-TV, etc. is realized
// through JavaScript files. The format of the scripts is described below.
//
// The scripts are identified through domain names (e.g. commons.wikimedia.org),
// which should be unique for each script.

// The version() function should return the version of LXiMediaCenter that this
// script is developed for.
function version()              { return "0.3.0"; }

// The name() function should return the user readable name for the content
// provider that this script provides.
function name()                 { return "Example"; }

// The targetAudience function should return a space separated list of target
// audiences based on geographical location in the format
// "<Country>[/<Location>]" (e.g. "NL" or "UK/LON"), the string "*" can be used
// to indicate a global audience.
function audience()             { return "*"; }

// The listItems function should return the media items for the site. The
// function should return an array, where each item is an array with the
// following elements: [ <name>, <type> ]. name is the user-visible name of the
// stream and type is one of { "Audio", "Video", "Image", "Dir" }.
function listItems(path)
{
  if (path == "/")
  {
    var items =
      [ ["Live stream", "Audio"] ];

    return items;
  }
  else
    return [];
}

// The icon function should return a base64 encoded string representing a
// 128x128 PNG encoded icon for the specified ID.
function icon(name)
{ 
  return some_png;
}

// The streamLocation function should return the URL for a streamid.
function streamLocation(name)
{
  if (name == "Live stream")
    return "http://streams.somesite.somedomain:8100/";
}

// Icons may be defined here, use the "Add icon" feature to convert icons to the
// right format.
var some_png = "<base64 encoded icon>";
