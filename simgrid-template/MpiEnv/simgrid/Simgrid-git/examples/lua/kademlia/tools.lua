-- Copyright (c) 2012, 2014. The SimGrid Team.
-- All rights reserved.

-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the license (GNU LGPL) which comes with this package.

function bxor (a,b)
  local r = 0
  for i = 0, 31 do
    local x = a / 2 + b / 2
    if x ~= math.floor (x) then
      r = r + 2^i
    end
    a = math.floor (a / 2)
    b = math.floor (b / 2)
  end
  return r
end

function index_of(table, o)
	for i,v in pairs(table) do
		if v == o then
			return i;
		end
	end
	return -1
end
function is_integer(x)
return math.floor(x)==x
end