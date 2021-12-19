-- Copyright (c) 2011, 2013-2014. The SimGrid Team.
-- All rights reserved.

-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the license (GNU LGPL) which comes with this package.

simgrid.platf.open();

simgrid.platf.AS_open{id="AS0",mode="Full"}; 

simgrid.platf.host_new{id="Tremblay",power=98095000};
simgrid.platf.host_new{id="Jupiter",power=76296000};
simgrid.platf.host_new{id="Fafard",power=76296000};
simgrid.platf.host_new{id="Ginette",power=48492000};
simgrid.platf.host_new{id="Bourassa",power=48492000};

-- create Links
for i=10,0,-1 do
    simgrid.platf.link_new{id=i,bandwidth=252750+ i*768,latency=0.000270544+i*0.087};  
end

simgrid.platf.route_new{src="Tremblay",dest="Jupiter",links="1"};
simgrid.platf.route_new{src="Tremblay",dest="Fafard",links="0,1,2,3,4,8"};
simgrid.platf.route_new{src="Tremblay",dest="Ginette",links="3,4,5"};
simgrid.platf.route_new{src="Tremblay",dest="Bourassa",links="0,1,3,2,4,6,7"};

simgrid.platf.route_new{src="Jupiter",dest="Tremblay",links="1"};
simgrid.platf.route_new{src="Jupiter",dest="Fafard",links="0,1,2,3,4,8,9"};
simgrid.platf.route_new{src="Jupiter",dest="Ginette",links="3,4,5,9"};
simgrid.platf.route_new{src="Jupiter",dest="Bourassa",links="0,1,2,3,4,6,7,9"};
 
simgrid.platf.route_new{src="Fafard",dest="Tremblay",links="0,1,2,3,4,8"};
simgrid.platf.route_new{src="Fafard",dest="Jupiter",links="0,1,2,3,4,8,9"};
simgrid.platf.route_new{src="Fafard",dest="Ginette",links="0,1,2,5,8"};
simgrid.platf.route_new{src="Fafard",dest="Bourassa",links="6,7,8"};
  
simgrid.platf.route_new{src="Ginette",dest="Tremblay",links="3,4,5"};
simgrid.platf.route_new{src="Ginette",dest="Jupiter",links="3,4,5,9"};
simgrid.platf.route_new{src="Ginette",dest="Fafard",links="0,1,2,5,8"};
simgrid.platf.route_new{src="Ginette",dest="Bourassa",links="0,1,2,5,6,7"};

simgrid.platf.route_new{src="Bourassa",dest="Tremblay",links="0,1,3,2,4,6,7"};
simgrid.platf.route_new{src="Bourassa",dest="Jupiter",links="0,1,2,3,4,6,7,9"};
simgrid.platf.route_new{src="Bourassa",dest="Fafard",links="6,7,8"};
simgrid.platf.route_new{src="Bourassa",dest="Ginette",links="0,1,2,5,6,7"};
simgrid.platf.AS_close();

simgrid.platf.close();
  
