jQuery.fn.quberTip = function (options) {
	var defaults = {
		speed: 500,
		xOffset: 10,
		yOffset: 10
	};
	var options = $.extend(defaults, options);
	return this.each(function () {
		var $this = jQuery(this);
		if ($this.attr('data-title') != undefined) {
			//Pass the title to a variable and then remove it from DOM
			if ($this.attr('data-title') != '') {
				var tipTitle = ($this.attr('data-title'));
			} else {
				var tipTitle = 'QuberTip';
			}
			//Remove title attribute
			$this.removeAttr('data-title');
			$(this).hover(function (e) {
			}, function () {
				//Remove the tooltip from the DOM
			});
		}
	});
};