
// Translations for en_US
i18n_register({"plural": function(n) { return n == 1 ? 0 : 1; }, "catalog": {}});


/* app.js */
//console.log("Begin loading app.js");
/* Create irCameraView class */
//console.log("Creating custom view class");
/* Start Splunkiness */ 
//console.log("Start interfaces");
require([
		"underscore",
		"backbone",
		"splunkjs/mvc",
	    "splunkjs/mvc/searchmanager",
	    "splunkjs/mvc/simplesplunkview",
	    "/static/app/irCameraView/irCameraView.js",
	    "splunkjs/mvc/simplexml/ready!"
	    ], function ( 
	    // Keep these variables in the same order as the libraries above:
		    _,
		    Backbone,
		    mvc,
		    SearchManager,
		    SimpleSplunkView,
		    irCameraView
		) {
//		console.log("Accessing frameSearch");
		var search = new SearchManager({
			id: "frameSearch",
			earliest_time: "rt",
			latest_time: "rt",
			rt_queue_size: 5,
			cache: false,
/*			preview: true,
			auto_cancel: 0,
			auto_finalize_ec: 0,
			auto_pause: 0,
*/			
			search_mode: "realtime",
			cancelOnUnload: false,
			search: 'index=irevents | fields + _time, _raw | eval timestamp=strftime(_time, "%+"), ts=_time | stats latest(*) as *, latest(_*) as _*'
		});
	    // Render viewport
//	    console.log("Create viewport");
	    var viewport = new irCameraView({
	    	id: "irCameraView",
	    	managerid: "frameSearch",
	    	el: $("#viewPort")
	    }).render();
	    search.on('search:failed', function(properties) {
	    	console.log("Search failed");
	    });

	    search.on('search:done', function(properties) {
	    	console.log("Search is done, WTF?", properties);
	    	viewport.render();
	    });
});