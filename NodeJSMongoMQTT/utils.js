if (!String.prototype.startsWith) {
    String.prototype.startsWith = function(searchString, position){
        position = position || 0;
        return this.substr(position, searchString.length) === searchString;
  };
}


if (!String.prototype.endsWith) {
	String.prototype.endsWith = function(searchString, position) {
		position = position || this.length;
		position -= searchString.length;
		
		var lastIndex = this.indexOf(searchString, position);
		return lastIndex !== -1 && lastIndex === position;
	};
}

if (!String.prototype.formatUsingObject) {
	String.prototype.formatUsingObject = function(matches) {
		return (this ||  '').replace(/(^|[^\{])\{(.*?)\}/g, function(st, p1, p2) {
	        var xobj  = matches;

	        p2.split(/\./).forEach(function(x) {
	            if (xobj) xobj = xobj[x];
	        });

	        return p1 + (xobj || '').toString();
	    }).replace(/&#7b;(.*?)&#7d;/g, function(st, p2) {
	        var xobj  = matches;

	        p2.split(/\./).forEach(function(x) {
	            if (xobj) xobj = xobj[x];
	        });

	        return encodeURIComponent((xobj || '').toString());
	    });
	}
}