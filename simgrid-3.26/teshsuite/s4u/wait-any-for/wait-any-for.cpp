#include <cstdlib>
#include <iostream>
#include <string>
#include <simgrid/s4u.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(meh, "meh");

static void worker()
{
  auto mbox = simgrid::s4u::Mailbox::by_name("meh");
  int input1 = 42;
  int input2 = 51;

  XBT_INFO("Sending and receiving %d and %d asynchronously", input1, input2);

  auto put1 = mbox->put_async(&input1, 1000 * 1000 * 500);
  auto put2 = mbox->put_async(&input2, 1000 * 1000 * 1000);

  int * out1;
  auto get1 = mbox->get_async((void**)&out1);

  int * out2;
  auto get2 = mbox->get_async((void**)&out2);

  XBT_INFO("All comms have started");
  std::vector<simgrid::s4u::CommPtr> comms = {put1, put2, get1, get2};

  while (!comms.empty()) {
    int index = simgrid::s4u::Comm::wait_any_for(&comms, 0.5);
    if (index < 0)
      XBT_INFO("wait_any_for: Timeout reached");
    else {
      XBT_INFO("wait_any_for: A comm finished (index=%d, #comms=%zu)", index, comms.size());
      comms.erase(comms.begin() + index);
    }
  }

  XBT_INFO("All comms have finished");
  XBT_INFO("Got %d and %d", *out1, *out2);
}

int main(int argc, char* argv[])

{
  simgrid::s4u::Engine e(&argc, argv);
  e.load_platform(argv[1]);
  simgrid::s4u::Actor::create("worker", simgrid::s4u::Host::by_name("Tremblay"), worker);
  e.run();
  return 0;
}
