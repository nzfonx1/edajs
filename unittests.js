/* UnitTesting with nodeunit! */

var edajs = require("./build/Release/edajs");

exports.testLogout = function(test){
    edajs.FixLogout();
    current_user = edajs.FixGetCurrentUser();
    test.equal(current_user, "", "No user should be currently logged in.")
    test.done();
};


exports.testLogin = function(test){
    edajs.FixLogin("sia", "sia");
    current_user = edajs.FixGetCurrentUser();
    test.equal(current_user['username'], "SIA", "User sia could not log in.")
    test.done();
};