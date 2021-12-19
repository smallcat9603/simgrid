-- Copyright (c) 2011, 2013-2014. The SimGrid Team.
-- All rights reserved.

-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the license (GNU LGPL) which comes with this package.

 dofile 'master.lua'
 dofile 'slave.lua' 

  simgrid.init_application();
  --Set Application
   simgrid.host.set_function{host="Tremblay",fct="Master",args="20,550000000,1000000,4"};
   simgrid.host.set_function{host="Jupiter",fct="Slave",args="1"};
   simgrid.host.set_function{host="Fafard",fct="Slave",args="2"};
   simgrid.host.set_function{host="Ginette",fct="Slave",args="3"};
   simgrid.host.set_function{host="Bourassa",fct="Slave",args="0"};   
