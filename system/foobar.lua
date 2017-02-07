
print("listing gpu first")
for k,v in pairs(component.list("gpu", true)) do
    print("key: ", k)
    print("value: ", v)
end

print("listing all")
for k,v in pairs(component.list("gpu", true)) do
    print(k, v)
end


