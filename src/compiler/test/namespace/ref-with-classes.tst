/*
  	Test namespace qualified variable access
 */
use strict

{
	use default namespace "Embedthis.com"

	class Shape {

		public static var classVar: Number = 1

		public function Shape() {
		}

		public static function hello() : Number {
			return 7
		}
	}
}

var b : "Embedthis.com"::Shape

x = "Embedthis.com"::Shape.hello()
assert(x == 7)
assert("Embedthis.com"::Shape.classVar == 1)


use namespace "Embedthis.com"

//  Test access to class in a package after importing

var a : Shape = new Shape
var c : Shape = new "Embedthis.com"::Shape
var x : Number

x = Shape.hello()
assert(x == 7)

var "anySpace"::z: Number = 3
use namespace "anySpace"
assert(z == 3)
