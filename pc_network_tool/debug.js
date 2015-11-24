
/* 
 * Simple logging library that is a wrapper to console.log
 * useful if we have to do some fancy logging in the future
 */
exports.log = function(){
  console.log.apply(this, arguments);
};
