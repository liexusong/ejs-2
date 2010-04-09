/*
  	Test function parameter validation and conversions
 */

/*
  	Test ...rest args
 */
function testRest(a, b, c, ...items)
{
	assert(a is Number)
	assert(b is String)
	assert(c is boolean)
	assert(items == "4,5,6")
	assert(items is Array)
	assert(items.length == 3)
}
testRest(1, "Hello World", true, 4, 5, 6)


/*
  	Test parameter type conversion
 */
function testWithTypes(a: Number, b: String, c: Boolean) 
{
	assert(a is Number)
	assert(b is String)
	assert(c is Boolean || c == undefined || c == null)
}

testWithTypes(1, "77", 1 == 7)
testWithTypes("1", 77, null)
testWithTypes(true, true, undefined)


/*
    Test passing null
 */
function fun(s: String = null): String {
    return typeOf(s)
}

assert(fun() == "Null")
assert(fun(null) == "Null")
assert(fun("hello") == "String")


//  Args casted to strings

function funNull(s: String~ = null): String {
    return typeOf(s)
}

assert(funNull(7) == "String")
assert(funNull(false) == "String")
assert(funNull(undefined) == "String")
