--!strict

local log = toluau.log
local require = toluau.require;

local Module = {}

local AnotherModule = require("anotherfile")


--log("test", "hxiasd", 123123)

function Module.entry()
	log("hello world")
end

function Module.testClass(fooClass)

	fooClass.IntMem = 100
	fooClass.StrMem = "world";
	fooClass:PrintIntMem()
	fooClass:SayHello("xixi")

	local Lib = require("FooStaticLib")

	local result = Lib.Add(1, 2);
	log(result)

	local Enum = require("FooEnum1")
	log("FooEnum.Bar", Enum.Bar)
	log("FooEnum.Foo", Enum.Foo)
	log("FooEnum.Invalid", Enum.Invalid)
end

function Module.testNew()

	local FooClass = require("FooClass")
	local foo = FooClass(200, "you")

	foo:PrintIntMem()
	foo:SayHello("haha")

end

return Module;