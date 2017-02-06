print("foobar home")
print("component", component)
print("component.list", component and component.list)
print("component.invoke", component and component.invoke)

component.list()
component.invoke()

print("client field", type(component.client))
