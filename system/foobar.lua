print("foobar home")
print("component", component, type(component))
print("component.list", component and component.list)
print("component.invoke", component and component.invoke)

component.list("gpu", true)
component.invoke()

local mt = getmetatable(component.list)
if mt then
    print("component.list has a meta table", mt)
    print("component.list/mt/.instance: " , mt.instance)
else
    print("component.list doesn't have a meta table")
end

