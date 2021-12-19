-- Copyright (c) 2011, 2014. The SimGrid Team.
-- All rights reserved.

-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the license (GNU LGPL) which comes with this package.

require("sim_splay")

function SPLAYschool()
  log:print("My ip is: "..job.me.ip())
  for i = 1,200 do
    log:print(i)
  end
--[[
  events.sleep(5)

  if job.me.ip() == job.nodes[1].ip then
    rpc.call(job.nodes[2], {"call_me", job.me.ip()})
  end
  events.sleep(5)
  os.exit()
  --]]
end

function call_me(from)
  log:print("I received an RPC from "..from)
end

events.thread("SPLAYschool")
start.loop()
log:print("Simulation finished")

