

local Module = {}

toluau.log("test", "hxiasd", 123123)

function Module.entry()
	toluau.log("hello world")
end

function Module.testClass(fooClass)

	fooClass.IntMem = 100
	fooClass.StrMem = "world";
	fooClass:PrintIntMem()
	fooClass:SayHello("xixi")
end

return Module;