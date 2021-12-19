-- Copyright (c) 2011, 2014. The SimGrid Team.
-- All rights reserved.

-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the license (GNU LGPL) which comes with this package.

-- This code creates 3 simgrid processes and verifies that the global values
-- in each Lua world are correctly cloned from maestro and become different

require("simgrid")

global_string = "A global string set by maestro"

-- Assigns to the global string the first argument and prints it
function set_global_string(...)

  global_string = arg[1]
  simgrid.info("Changing the global string")
  print_global()
end

-- Replaces the function please_dont_change_me() by set_global_string()
-- and calls it
function replace(...)

  simgrid.info("Overwriting function please_dont_replace_me()")
  please_dont_replace_me = set_global_string
  please_dont_replace_me(...)
end

-- Shows a hello message and prints the global string
function please_dont_replace_me(...)

  simgrid.info("Hello from please_dont_replace_me(). I'm lucky, I still exist in this state.")
  print_global()
end

-- Prints the value of global_string
function print_global()

  simgrid.info("Global string is '" .. global_string .. "'")
end

print_global()

simgrid.platform("../../platforms/small_platform.xml")
simgrid.application("deployment_duplicated_globals.xml")
simgrid.run()

