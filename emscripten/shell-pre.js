// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..
window.spp_backend_state_SC68= {
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_SC68["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_SC68);

var backend_SC68 = (function(Module) {