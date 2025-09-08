-- you must have psl.data file in this directory
-- you must have libctld.so in this directory

psl = require "libctld"
inspect = require "inspect"
p = psl.init("psl.dat")
result, err, msg = psl.parse(p, "google.com", 0)

print(inspect(result))
print(inspect(err))
print(inspect(msg))

