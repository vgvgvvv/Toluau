--!strict

local log = toluau.log
local require = toluau.require;

type Point = {x : number, y : number}
local p : Point = {x = 100, y = 200}

print(p.x, p.y);
print(p.z)

local Module = {}

local AnotherModule = require(anotherfile)



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

	local Enum = require("FooEnum")
	log("FooEnum.Bar", Enum.Bar)
	log("FooEnum.Foo", Enum.Foo)

end

function Module.testNew()

	local FooClass = require("FooClass")
	local foo = FooClass(200, "you")

	foo:PrintIntMem()
	foo:SayHello("haha")

end

return Module;