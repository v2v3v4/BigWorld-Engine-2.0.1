var bwVersion = "2.0";
var custName  = "BigWorld";
var bwDocName = "Unknown";

function loader()
{
	try { verifyCustomer(); }
	catch (e) { custName = "Unknown"; }

	imgURL = "http://try.bigworldtech.com/images/" + bwVersion + "/" + custName + "/" + bwDocName + "/bwlogo_bluebkg.png"

	document.getElementById("bwLogo").src = imgURL;
}

function verifyCustomer()
{
	var baseURI   = document.baseURI || document.URL;

	// Determine the document name
	gen_re = /doc\/generated_html\/([a-zA-Z0-9_]*)\/.*\.html/;
	docNameMatch = baseURI.match( gen_re );
	if (docNameMatch != null )
	{
		// Generated HTML documents

		// We expect 2 items in the list, the url match and the submatch
		// containing the doc name
		if (docNameMatch.length != 2)
		{
			bwDocName = "Unknown_GeneratedHtml";
		}
		else
		{
			// The second element should always be the document name.
			bwDocName = docNameMatch[ 1 ];
		}
	}
	else
	{
		// Static HTML documents

		var docNameStart = baseURI.lastIndexOf( "/" ) + 1;
		// If we're being viewed on a local filesystem, it's possible
		// that the seperators are '\'
		if (docNameStart <= 7)
		{
			docNameStart = baseURI.lastIndexOf( "\\" ) + 1;
		}

		var docNameLen   = baseURI.lastIndexOf( "." ) - docNameStart;
		bwDocName = baseURI.substr( docNameStart, docNameLen );
	}


	var re = /^.*svn\/customers\//;
	var customer = String( "$HeadURL: https://svn01.bigworldtech.com/svn/customers/Xingyulongying/2.0/current/bigworld/doc/generated_html/css/bigworld.js $" ).replace( re, "" );
	if (customer.match( "HeadURL" ) )
	{
		custName = "BigWorld";
		return;
	}
	var firstSep = customer.indexOf( "/");
	var firstVer = firstSep + 1;
	custName = customer.substr( 0, firstSep );
	if (custName == "")
	{
		custName = "Unknown";
	}

	var verLen = customer.indexOf( "/", firstVer ) - firstVer;
	bwVersion = customer.substr( firstVer, verLen )
	if (bwVersion == "")
	{
		bwVersion = "Unknown";
	}
}
