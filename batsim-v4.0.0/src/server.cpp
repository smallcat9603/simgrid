/**
 * @file server.cpp
 * @brief Contains functions related to the general orchestration of the simulation
 */

#include "server.hpp"

#include <string>
#include <set>
#include <memory>

#include <boost/algorithm/string.hpp>

#include <simgrid/s4u.hpp>

#include "context.hpp"
#include "ipp.hpp"
#include "network.hpp"
#include "jobs_execution.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(server, "server"); //!< Logging

using namespace std;

void server_process(BatsimContext * context)
{
    ServerData * data = new ServerData;
    data->context = context;

    // Let's tell the Decision process that the simulation is about to begin
    // (and that some data can be read from the data storage)
    context->proto_writer->append_simulation_begins(context->machines,
                                                    context->workloads,
                                                    context->config_json,
                                                    context->allow_compute_sharing,
                                                    context->allow_storage_sharing,
                                                    simgrid::s4u::Engine::get_clock());

    string send_buffer = context->proto_writer->generate_current_message(simgrid::s4u::Engine::get_clock());
    context->proto_writer->clear();

    simgrid::s4u::Actor::create("Scheduler REQ-REP", simgrid::s4u::this_actor::get_host(),
                                request_reply_scheduler_process,
                                context, send_buffer);
    data->sched_ready = false;

    // Let's prepare a handler map to react on events
    std::map<IPMessageType, std::function<void(ServerData *, IPMessage *)>> handler_map;
    handler_map[IPMessageType::JOB_SUBMITTED] = server_on_job_submitted;
    handler_map[IPMessageType::JOB_REGISTERED_BY_DP] = server_on_register_job;
    handler_map[IPMessageType::PROFILE_REGISTERED_BY_DP] = server_on_register_profile;
    handler_map[IPMessageType::JOB_COMPLETED] = server_on_job_completed;
    handler_map[IPMessageType::PSTATE_MODIFICATION] = server_on_pstate_modification;
    handler_map[IPMessageType::SCHED_EXECUTE_JOB] = server_on_execute_job;
    handler_map[IPMessageType::SCHED_CHANGE_JOB_STATE] = server_on_change_job_state;
    handler_map[IPMessageType::TO_JOB_MSG] = server_on_to_job_msg;
    handler_map[IPMessageType::FROM_JOB_MSG] = server_on_from_job_msg;
    handler_map[IPMessageType::SCHED_REJECT_JOB] = server_on_reject_job;
    handler_map[IPMessageType::SCHED_KILL_JOB] = server_on_kill_jobs;
    handler_map[IPMessageType::SCHED_CALL_ME_LATER] = server_on_call_me_later;
    handler_map[IPMessageType::SCHED_TELL_ME_ENERGY] = server_on_sched_tell_me_energy;
    handler_map[IPMessageType::SCHED_SET_JOB_METADATA] = server_on_set_job_metadata;
    handler_map[IPMessageType::SCHED_WAIT_ANSWER] = server_on_sched_wait_answer;
    handler_map[IPMessageType::WAIT_QUERY] = server_on_wait_query;
    handler_map[IPMessageType::SCHED_READY] = server_on_sched_ready;
    handler_map[IPMessageType::WAITING_DONE] = server_on_waiting_done;
    handler_map[IPMessageType::KILLING_DONE] = server_on_killing_done;
    handler_map[IPMessageType::SUBMITTER_HELLO] = server_on_submitter_hello;
    handler_map[IPMessageType::SUBMITTER_BYE] = server_on_submitter_bye;
    handler_map[IPMessageType::SWITCHED_ON] = server_on_switched;
    handler_map[IPMessageType::SWITCHED_OFF] = server_on_switched;
    handler_map[IPMessageType::END_DYNAMIC_REGISTER] = server_on_end_dynamic_register;
    handler_map[IPMessageType::CONTINUE_DYNAMIC_REGISTER] = server_on_continue_dynamic_register;
    handler_map[IPMessageType::EVENT_OCCURRED] = server_on_event_occurred;

    /* Currently, there is one job submtiter per input file (workload or workflow).
       As workflows use an inner workload, calling nb_static_workloads() should
       be enough. The dynamic job submitter (from the decision process) is not part
       of this count as it can finish then restart... */
    ServerData::SubmitterCounters job_counters;
    job_counters.expected_nb_submitters = context->workloads.nb_static_workloads();
    data->submitter_counters[SubmitterType::JOB_SUBMITTER] = job_counters;

    ServerData::SubmitterCounters event_counters;
    event_counters.expected_nb_submitters = static_cast<unsigned int>(context->event_lists.size());
    data->submitter_counters[SubmitterType::EVENT_SUBMITTER] = event_counters;

    data->jobs_to_be_deleted.clear();

    // Simulation loop
    while (!data->end_of_simulation_ack_received)
    {
        // Wait and receive a message from a node or the request-reply process...
        IPMessage * message = receive_message("server");
        XBT_DEBUG("Server received a message of type %s:",
                 ip_message_type_to_string(message->type).c_str());

        // Handle the message
        xbt_assert(handler_map.count(message->type) == 1,
                   "The server does not know how to handle message type %s.",
                   ip_message_type_to_string(message->type).c_str());
        auto handler_function = handler_map[message->type];
        handler_function(data, message);

        // Delete the message
        delete message;

        // Let's send a message to the scheduler if needed
        if (data->sched_ready &&                     // The scheduler must be ready
            !data->end_of_simulation_ack_received && // The simulation must NOT be finished
            mailbox_empty("server")                  // The server mailbox must be empty
            )
        {
            if (!context->proto_writer->is_empty()) // There is something to send to the scheduler
            {
                generate_and_send_message(data);
                if (!data->jobs_to_be_deleted.empty())
                {
                    data->context->workloads.delete_jobs(data->jobs_to_be_deleted,
                                                         context->garbage_collect_profiles);
                    data->jobs_to_be_deleted.clear();
                }
            }
            else // There is no event to send to the scheduler
            {
                // Check if the simulation is finished
                if (is_simulation_finished(data) &&
                    !data->end_of_simulation_sent)
                {
                    XBT_INFO("The simulation seems finished.");
                    data->context->proto_writer->append_simulation_ends(simgrid::s4u::Engine::get_clock());
                    generate_and_send_message(data);
                    data->end_of_simulation_sent = true;
                }
            }
        }

    } // end of while

    XBT_INFO("Simulation is finished!");

    // Is simulation also finished for the decision process?
    xbt_assert(data->end_of_simulation_sent, "Left simulation loop, but the SIMULATION_ENDS message has not been sent to the scheduler.");
    xbt_assert(data->end_of_simulation_ack_received, "Left simulation loop, but the decision process did not ACK the SIMULATION_ENDS message.");

    // Are there still pending actions in Batsim?
    xbt_assert(data->sched_ready, "Left simulation loop, but a call to the decision process is ongoing.");
    xbt_assert(data->nb_running_jobs == 0, "Left simulation loop, but some jobs are running.");
    xbt_assert(data->nb_switching_machines == 0, "Left simulation loop, but some machines are being switched.");
    xbt_assert(data->nb_killers == 0, "Left simulation loop, but some killer processes (used to kill jobs) are running.");
    xbt_assert(data->nb_waiters == 0, "Left simulation loop, but some waiter processes (used to manage the CALL_ME_LATER message) are running.");

    // Consistency
    xbt_assert(data->nb_completed_jobs == data->nb_submitted_jobs, "All submitted jobs have not been completed (either executed and finished, or rejected).");

    delete data;
}

void generate_and_send_message(ServerData * data)
{
    string send_buffer = data->context->proto_writer->generate_current_message(simgrid::s4u::Engine::get_clock());
    data->context->proto_writer->clear();

    simgrid::s4u::Actor::create("Scheduler REQ-REP", simgrid::s4u::this_actor::get_host(),
                                request_reply_scheduler_process,
                                data->context, send_buffer);
    data->sched_ready = false;
}

void server_on_submitter_hello(ServerData * data,
                               IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    xbt_assert(!data->end_of_simulation_sent,
               "A new submitter said hello but the simulation is finished... Aborting.");
    auto * message = static_cast<SubmitterHelloMessage *>(task_data->data);

    xbt_assert(data->submitters.count(message->submitter_name) == 0,
               "Invalid new submitter '%s': a submitter with the same name already exists!",
               message->submitter_name.c_str());

    ServerData::Submitter * submitter = new ServerData::Submitter;
    submitter->mailbox = message->submitter_name;
    submitter->should_be_called_back = message->enable_callback_on_job_completion;

    data->submitters[message->submitter_name] = submitter;

    SubmitterType submitter_type = message->submitter_type;
    ++data->submitter_counters[submitter_type].nb_submitters;

    XBT_DEBUG("New %s submitter said hello. Number of polite %s submitters: %d",
              submitter_type_to_string(submitter_type).c_str(),
              submitter_type_to_string(submitter_type).c_str(),
              data->submitter_counters[submitter_type].nb_submitters);

}

void server_on_submitter_bye(ServerData * data,
                             IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<SubmitterByeMessage *>(task_data->data);

    xbt_assert(data->submitters.count(message->submitter_name) == 1);
    delete data->submitters[message->submitter_name];
    data->submitters.erase(message->submitter_name);

    SubmitterType submitter_type = message->submitter_type;

    ++data->submitter_counters[submitter_type].nb_submitters_finished;

    XBT_DEBUG("%s submitter said goodbye. Number of finished %s submitters: %d",
              submitter_type_to_string(submitter_type).c_str(),
              submitter_type_to_string(submitter_type).c_str(),
              data->submitter_counters[submitter_type].nb_submitters);

    if (submitter_type == SubmitterType::EVENT_SUBMITTER)
    {
        if(data->submitter_counters[submitter_type].nb_submitters_finished == data->submitter_counters[submitter_type].expected_nb_submitters)
        {
            data->context->proto_writer->append_notify("no_more_external_event_to_occur", simgrid::s4u::Engine::get_clock());
        }
    }
    else if (submitter_type == SubmitterType::JOB_SUBMITTER)
    {
        if(data->submitter_counters[submitter_type].nb_submitters_finished == data->submitter_counters[submitter_type].expected_nb_submitters)
        {
          data->context->proto_writer->append_notify("no_more_static_job_to_submit", simgrid::s4u::Engine::get_clock());
        }

        if (message->is_workflow_submitter)
        {
            data->nb_workflow_submitters_finished++;
        }
    }
}

void server_on_job_completed(ServerData * data,
                             IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<JobCompletedMessage *>(task_data->data);

    if (data->origin_of_jobs.count(message->job->id) == 1)
    {
        // Let's call the submitter which submitted the job back
        SubmitterJobCompletionCallbackMessage * msg = new SubmitterJobCompletionCallbackMessage;
        msg->job_id = message->job->id;

        ServerData::Submitter * submitter = data->origin_of_jobs.at(message->job->id);
        dsend_message(submitter->mailbox, IPMessageType::SUBMITTER_CALLBACK, static_cast<void*>(msg));

        data->origin_of_jobs.erase(message->job->id);
    }

    data->nb_running_jobs--;
    xbt_assert(data->nb_running_jobs >= 0);
    data->nb_completed_jobs++;
    xbt_assert(data->nb_completed_jobs + data->nb_running_jobs <= data->nb_submitted_jobs);
    auto job = message->job;

    XBT_INFO("Job %s has COMPLETED. %d jobs completed so far",
             job->id.to_cstring(), data->nb_completed_jobs);

    data->context->proto_writer->append_job_completed(message->job->id.to_string(),
                                                      job_state_to_string(job->state),
                                                      job->allocation.to_string_hyphen(" "),
                                                      job->return_code,
                                                      simgrid::s4u::Engine::get_clock());

    data->context->jobs_tracer.write_job(job);
    data->jobs_to_be_deleted.push_back(message->job->id);
}

void server_on_job_submitted(ServerData * data,
                             IPMessage * task_data)
{
    // Ignore all submissions if -k was specified and all workflows have completed
    if ((data->context->workflows.size() != 0) && (data->context->terminate_with_last_workflow) &&
        (data->nb_workflow_submitters_finished == data->context->workflows.size()))
    {
        XBT_INFO("Ignoring Job due to -k command-line option");
        return;
    }

    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<JobSubmittedMessage *>(task_data->data);

    xbt_assert(data->submitters.count(message->submitter_name) == 1);

    ServerData::Submitter * submitter = data->submitters.at(message->submitter_name);
    for (JobPtr & job : message->jobs)
    {
        if (submitter->should_be_called_back)
        {
            xbt_assert(data->origin_of_jobs.count(job->id) == 0);
            data->origin_of_jobs[job->id] = submitter;
        }

        // Let's retrieve the Job from memory (or add it into memory if it is dynamic)
        XBT_DEBUG("Job received: %s", job->id.to_cstring());

        XBT_DEBUG("Workloads: %s", data->context->workloads.to_string().c_str());

        // Update control information
        job->state = JobState::JOB_STATE_SUBMITTED;
        ++data->nb_submitted_jobs;
        XBT_INFO("Job %s SUBMITTED. %d jobs submitted so far", job->id.to_cstring(), data->nb_submitted_jobs);

        string job_json_description, profile_json_description;

        if (!data->context->redis_enabled)
        {
            job_json_description = job->json_description;
            if (data->context->submission_forward_profiles)
            {
                profile_json_description = job->profile->json_description;
            }
        }

        data->context->proto_writer->append_job_submitted(job->id.to_string(),
                                                          job_json_description,
                                                          profile_json_description,
                                                          simgrid::s4u::Engine::get_clock());
    }
}


void server_on_event_machine_unavailable(ServerData * data,
                                         const Event * event)
{
    auto * event_data = static_cast<MachineAvailabilityEventData*>(event->data);
    IntervalSet machines = event_data->machine_ids;
    if(machines.size() > 0)
    {
        for (auto machine_it = machines.elements_begin();
             machine_it != machines.elements_end();
             ++machine_it)
        {
            const int machine_id = *machine_it;
            Machine * machine = data->context->machines[machine_id];

            xbt_assert(machine->state != MachineState::UNAVAILABLE,
                       "The making of machine %d ('%s') unavailable was requested but "
                       "this machine was already in UNAVAILABLE state.",
                       machine->id, machine->name.c_str());

            machine->update_machine_state(MachineState::UNAVAILABLE);
        }
        // Notify the decision process that some machines have become unavailable
        data->context->proto_writer->append_notify_resource_event("event_machine_unavailable",
                                                                  machines,
                                                                  simgrid::s4u::Engine::get_clock());
    }
}

void server_on_event_machine_available(ServerData * data,
                                       const Event * event)
{
    auto * event_data = static_cast<MachineAvailabilityEventData*>(event->data);
    IntervalSet machines = event_data->machine_ids;
    if(machines.size() > 0)
    {
        for (auto machine_it = machines.elements_begin();
             machine_it != machines.elements_end();
             ++machine_it)
        {
            Machine * machine = data->context->machines[*machine_it];

            xbt_assert(machine->state == MachineState::UNAVAILABLE,
                       "The making of machine %d ('%s') available was requested but "
                       "this machine was not in UNAVAILABLE state (current state: %s).",
                       machine->id, machine->name.c_str(), machine_state_to_string(machine->state).c_str());

            if (machine->jobs_being_computed.empty())
            {
                machine->update_machine_state(MachineState::IDLE);
            }
            else
            {
                machine->update_machine_state(MachineState::COMPUTING);
            }
        }
        // Notify the decision process that some machines have become available
        data->context->proto_writer->append_notify_resource_event("event_machine_available",
                                                                  machines,
                                                                  simgrid::s4u::Engine::get_clock());
    }
}

void server_on_event_generic(ServerData * data,
                             const Event * event)
{

    // Just forward the json object
    auto * event_data = static_cast<GenericEventData*>(event->data);
    data->context->proto_writer->append_notify_generic_event(event_data->json_desc_str,
                                                             simgrid::s4u::Engine::get_clock());
}

void server_on_event_occurred(ServerData * data,
                              IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<EventOccurredMessage *>(task_data->data);

    for (const Event * event : message->occurred_events)
    {
        switch(event->type)
        {
        case EventType::EVENT_MACHINE_AVAILABLE:
            server_on_event_machine_available(data, event);
            break;
        case EventType::EVENT_MACHINE_UNAVAILABLE:
            server_on_event_machine_unavailable(data, event);
            break;
        case EventType::EVENT_GENERIC:
            server_on_event_generic(data, event);
            break;
        }
    }
}

void server_on_pstate_modification(ServerData * data,
                                   IPMessage * task_data)
{
    xbt_assert(data->context->energy_used,
               "Receiving a pstate modification request, which is forbidden as "
               "Batsim has not been launched with energy support "
               "(cf. batsim --help).");

    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<PStateModificationMessage *>(task_data->data);

    data->context->current_switches.add_switch(message->machine_ids, message->new_pstate);
    data->context->energy_tracer.add_pstate_change(simgrid::s4u::Engine::get_clock(), message->machine_ids,
                                                   message->new_pstate);

    // Let's quickly check whether this is a switchON or a switchOFF
    // Unknown transition states will be set to -42.
    int transition_state = -42;
    Machine * first_machine = data->context->machines[message->machine_ids.first_element()];
    if (first_machine->pstates[message->new_pstate] == PStateType::COMPUTATION_PSTATE)
    {
        transition_state = -1; // means we are switching to a COMPUTATION_PSTATE
    }
    else if (first_machine->pstates[message->new_pstate] == PStateType::SLEEP_PSTATE)
    {
        transition_state = -2; // means we are switching to a SLEEP_PSTATE
    }


    // The pstate is set to an invalid one to know the machines are in transition.
    data->context->pstate_tracer.add_pstate_change(simgrid::s4u::Engine::get_clock(), message->machine_ids,
                                                   transition_state);

    // Let's mark that some switches have been requested
    data->context->nb_grouped_switches++;
    data->context->nb_machine_switches += message->machine_ids.size();

    for (auto machine_it = message->machine_ids.elements_begin();
         machine_it != message->machine_ids.elements_end();
         ++machine_it)
    {
        const int machine_id = *machine_it;
        Machine * machine = data->context->machines[machine_id];
        int curr_pstate = machine->host->get_pstate();

        if (machine->pstates[curr_pstate] == PStateType::COMPUTATION_PSTATE)
        {
            if (machine->pstates[message->new_pstate] == PStateType::COMPUTATION_PSTATE)
            {
                XBT_INFO("Switching machine %d ('%s') pstate : %d -> %d.", machine->id,
                         machine->name.c_str(), curr_pstate, message->new_pstate);
                machine->host->set_pstate(message->new_pstate);
                xbt_assert(machine->host->get_pstate() == message->new_pstate);

                IntervalSet all_switched_machines;
                if (data->context->current_switches.mark_switch_as_done(machine->id, message->new_pstate,
                                                                        all_switched_machines,
                                                                        data->context))
                {
                    data->context->proto_writer->append_resource_state_changed(all_switched_machines,
                                                                               std::to_string(message->new_pstate),
                                                                               simgrid::s4u::Engine::get_clock());
                }
            }
            else if (machine->pstates[message->new_pstate] == PStateType::SLEEP_PSTATE)
            {
                machine->update_machine_state(MachineState::TRANSITING_FROM_COMPUTING_TO_SLEEPING);

                string pname = "switch ON " + to_string(machine_id);
                simgrid::s4u::Actor::create(pname.c_str(), machine->host, switch_off_machine_process,
                                            data->context, machine_id, message->new_pstate);

                ++data->nb_switching_machines;
            }
            else
            {
                XBT_ERROR("Switching from a communication pstate to an invalid pstate on machine %d ('%s') : %d -> %d",
                          machine->id, machine->name.c_str(), curr_pstate, message->new_pstate);
            }
        }
        else if (machine->pstates[curr_pstate] == PStateType::SLEEP_PSTATE)
        {
            xbt_assert(machine->pstates[message->new_pstate] == PStateType::COMPUTATION_PSTATE,
                    "Switching from a sleep pstate to a non-computation pstate on machine %d ('%s') : %d -> %d, which is forbidden",
                    machine->id, machine->name.c_str(), curr_pstate, message->new_pstate);

            machine->update_machine_state(MachineState::TRANSITING_FROM_SLEEPING_TO_COMPUTING);

            string pname = "switch OFF " + to_string(machine_id);
            simgrid::s4u::Actor::create(pname.c_str(), machine->host, switch_on_machine_process,
                                        data->context, machine_id, message->new_pstate);

            ++data->nb_switching_machines;
        }
        else
        {
            XBT_ERROR("Machine %d ('%s') has an invalid pstate : %d", machine->id, machine->name.c_str(), curr_pstate);
        }
    }

    if (data->context->trace_machine_states)
    {
        data->context->machine_state_tracer.write_machine_states(simgrid::s4u::Engine::get_clock());
    }
}

void server_on_waiting_done(ServerData * data,
                            IPMessage * task_data)
{
    (void) task_data;
    data->context->proto_writer->append_requested_call(simgrid::s4u::Engine::get_clock());
    --data->nb_waiters;
}

void server_on_sched_ready(ServerData * data,
                           IPMessage * task_data)
{
    (void) task_data;
    data->sched_ready = true;

    if (data->end_of_simulation_sent)
    {
        data->end_of_simulation_ack_received = true;
    }
}

void server_on_sched_wait_answer(ServerData * data,
                                 IPMessage * task_data)
{
    (void) data;
    auto * message = new SchedWaitAnswerMessage;
    *message = *(static_cast<SchedWaitAnswerMessage *>(task_data->data)); // is this necessary?

    //    Submitter * submitter = origin_of_wait_queries.at({message->nb_resources,message->processing_time});
    dsend_message(message->submitter_name, IPMessageType::SCHED_WAIT_ANSWER, static_cast<void*>(message));
    //    origin_of_wait_queries.erase({message->nb_resources,message->processing_time});
}

void server_on_sched_tell_me_energy(ServerData * data,
                                    IPMessage * task_data)
{
    (void) task_data;
    xbt_assert(data->context->energy_used,
               "Received a request about the energy consumption of the "
               "machines but energy simulation is not enabled. "
               "Try --help to enable it.");
    double total_consumed_energy = static_cast<double>(data->context->machines.total_consumed_energy(data->context));
    data->context->proto_writer->append_answer_energy(total_consumed_energy, simgrid::s4u::Engine::get_clock());
}

void server_on_wait_query(ServerData * data,
                          IPMessage * task_data)
{
    (void) data;
    (void) task_data;
    //WaitQueryMessage * message = (WaitQueryMessage *) task_data->data;

    //    XBT_INFO("received : %s , %s\n", to_string(message->nb_resources).c_str(), to_string(message->processing_time).c_str());
    xbt_assert(false, "Unimplemented! TODO");

    //Submitter * submitter = submitters.at(message->submitter_name);
    //origin_of_wait_queries[{message->nb_resources,message->processing_time}] = submitter;
}

void server_on_switched(ServerData * data,
                        IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<SwitchMessage *>(task_data->data);

    xbt_assert(data->context->machines.exists(message->machine_id));
    Machine * machine = data->context->machines[message->machine_id];
    (void) machine; // Avoids a warning if assertions are ignored
    xbt_assert(machine->host->get_pstate() == message->new_pstate);

    IntervalSet all_switched_machines;
    if (data->context->current_switches.mark_switch_as_done(message->machine_id, message->new_pstate,
                                                            all_switched_machines, data->context))
    {
        if (data->context->trace_machine_states)
        {
            data->context->machine_state_tracer.write_machine_states(simgrid::s4u::Engine::get_clock());
        }

        data->context->proto_writer->append_resource_state_changed(all_switched_machines,
                                                                   std::to_string(message->new_pstate),
                                                                   simgrid::s4u::Engine::get_clock());
    }

    --data->nb_switching_machines;
}

void server_on_killing_done(ServerData * data,
                            IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<KillingDoneMessage *>(task_data->data);

    map<string, BatTask *> jobs_progress_str;
    vector<string> really_killed_job_ids_str;
    vector<string> job_ids_str;
    job_ids_str.reserve(message->jobs_ids.size());

    // manage job Id list
    for (const JobIdentifier & job_id : message->jobs_ids)
    {
        job_ids_str.push_back(job_id.to_string());

        // store job progress from BatTask tree in str
        jobs_progress_str[job_id.to_string()] = message->jobs_progress[job_id];

        const auto job = data->context->workloads.job_at(job_id);
        if ( job->state == JobState::JOB_STATE_COMPLETED_KILLED)
        {
            data->nb_running_jobs--;
            xbt_assert(data->nb_running_jobs >= 0);
            data->nb_completed_jobs++;
            xbt_assert(data->nb_completed_jobs + data->nb_running_jobs <= data->nb_submitted_jobs);

            really_killed_job_ids_str.push_back(job_id.to_string());

            // also add a job complete message for the jobs that have really been
            // killed
            data->context->proto_writer->append_job_completed(
                job->id.to_string(),
                job_state_to_string(job->state),
                job->allocation.to_string_hyphen(" "),
                job->return_code,
                simgrid::s4u::Engine::get_clock());

            data->context->jobs_tracer.write_job(job);
            data->jobs_to_be_deleted.push_back(job->id);
        }
    }

    if (message->acknowledge_kill_on_protocol)
    {
        // The kills have been asked by the decision process
        XBT_INFO("Jobs {%s} have been killed (the following ones have REALLY been killed: {%s})",
                 boost::algorithm::join(job_ids_str, ",").c_str(),
                 boost::algorithm::join(really_killed_job_ids_str, ",").c_str());

        data->context->proto_writer->append_job_killed(job_ids_str, jobs_progress_str, simgrid::s4u::Engine::get_clock());
    }

    --data->nb_killers;
}

void server_on_end_dynamic_register(ServerData * data,
                                  IPMessage * task_data)
{
    (void) task_data;

    data->context->registration_sched_finished = true;
}

void server_on_continue_dynamic_register(ServerData * data,
                                  IPMessage * task_data)
{
    (void) task_data;
    data->context->registration_sched_finished = false;
}

void server_on_register_job(ServerData * data,
                          IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<JobRegisteredByDPMessage *>(task_data->data);
    auto job = message->job;

    // Let's update global states
    ++data->nb_submitted_jobs;

    if (data->context->registration_sched_ack)
    {
        // TODO Sleep until submit time is reached before sending the ack (JOB_SUBMITTED)
        string job_json_description, profile_json_description;

        if (!data->context->redis_enabled)
        {
            job_json_description = job->json_description;
            if (data->context->submission_forward_profiles)
            {
                profile_json_description = job->profile->json_description;
            }
        }

        data->context->proto_writer->append_job_submitted(job->id.to_string(),
                                                          job_json_description,
                                                          profile_json_description,
                                                          simgrid::s4u::Engine::get_clock());
    }
}

void server_on_register_profile(ServerData * data,
                          IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<ProfileRegisteredByDPMessage *>(task_data->data);
    (void) data;
    (void) message;
    // TODO: remove me?
}

void server_on_set_job_metadata(ServerData * data,
                                IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<SetJobMetadataMessage *>(task_data->data);

    JobIdentifier job_identifier = JobIdentifier(message->job_id);
    if (!(data->context->workloads.job_is_registered(job_identifier)))
    {
        xbt_die("The job '%s' does not exist, cannot set its metadata", message->job_id.to_cstring());
    }

    auto job = data->context->workloads.job_at(job_identifier);
    job->metadata = message->metadata;
    XBT_DEBUG("Metadata of job '%s' has been set", message->job_id.to_cstring());
}

void server_on_change_job_state(ServerData * data,
                                IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<ChangeJobStateMessage *>(task_data->data);

    if (!(data->context->workloads.job_is_registered(message->job_id)))
    {
        xbt_die("The job '%s' does not exist.", message->job_id.to_cstring());
    }
    auto job = data->context->workloads.job_at(message->job_id);

    XBT_INFO("Change job state: Job %s to state %s",
             job->id.to_cstring(),
             message->job_state.c_str());

    JobState new_state = job_state_from_string(message->job_state);

    switch (job->state)
    {
    case JobState::JOB_STATE_SUBMITTED:
        switch (new_state)
        {
        case JobState::JOB_STATE_RUNNING:
            job->starting_time = static_cast<long double>(simgrid::s4u::Engine::get_clock());
            data->nb_running_jobs++;
            xbt_assert(data->nb_running_jobs <= data->nb_submitted_jobs);
            break;
        case JobState::JOB_STATE_REJECTED:
            data->nb_completed_jobs++;
            xbt_assert(data->nb_completed_jobs + data->nb_running_jobs <= data->nb_submitted_jobs);
            data->context->jobs_tracer.write_job(job);
            data->jobs_to_be_deleted.push_back(message->job_id);
            break;
        default:
            xbt_assert(false,
                       "Invalid requested job state modification to '%s': can only be running or rejected.",
                       job_state_to_string(new_state).c_str());
        }
        break;
    case JobState::JOB_STATE_RUNNING:
        switch (new_state)
        {
        case JobState::JOB_STATE_COMPLETED_SUCCESSFULLY:
        case JobState::JOB_STATE_COMPLETED_FAILED:
        case JobState::JOB_STATE_COMPLETED_WALLTIME_REACHED:
        case JobState::JOB_STATE_COMPLETED_KILLED:
            job->runtime = static_cast<long double>(simgrid::s4u::Engine::get_clock()) - job->starting_time;
            data->nb_running_jobs--;
            xbt_assert(data->nb_running_jobs >= 0);
            data->nb_completed_jobs++;
            xbt_assert(data->nb_completed_jobs + data->nb_running_jobs <= data->nb_submitted_jobs);
            data->context->jobs_tracer.write_job(job);
            data->jobs_to_be_deleted.push_back(message->job_id);
            break;
        default:
            xbt_assert(false,
                       "Can only change the state of a running job to completed "
                       "(successfully, failed, and killed). State was %s",
                       job_state_to_string(job->state).c_str());
        }
        break;
    default:
        xbt_assert(false,
                   "Can only change the state of a submitted or running job. State was %s",
                   job_state_to_string(job->state).c_str());
    }

    job->state = new_state;
}

void server_on_to_job_msg(ServerData * data,
                          IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<ToJobMessage *>(task_data->data);

    if (!(data->context->workloads.job_is_registered(message->job_id)))
    {
        xbt_die("The job '%s' does not exist, cannot send a message to that job.", message->job_id.to_cstring());
    }
    auto job = data->context->workloads.job_at(message->job_id);

    XBT_INFO("Send message to job: Job '%s' message='%s'",
             job->id.to_cstring(),
             message->message.c_str());

    job->incoming_message_buffer.push_back(message->message);
}

void server_on_from_job_msg(ServerData * data,
                          IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<FromJobMessage *>(task_data->data);

    auto job = data->context->workloads.job_at(message->job_id);

    XBT_INFO("Send message to scheduler: Job %s", job->id.to_cstring());

    data->context->proto_writer->append_from_job_message(message->job_id.to_string(),
                                                         message->message,
                                                         simgrid::s4u::Engine::get_clock());
}

void server_on_reject_job(ServerData * data,
                          IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<JobRejectedMessage *>(task_data->data);

    if (!(data->context->workloads.job_is_registered(message->job_id)))
    {
        xbt_die("Job '%s' does not exist.", message->job_id.to_cstring());
    }

    auto job = data->context->workloads.job_at(message->job_id);
    (void) job; // Avoids a warning if assertions are ignored
    xbt_assert(job->state == JobState::JOB_STATE_SUBMITTED,
               "Invalid rejection received: job '%s' cannot be rejected at the present time. "
               "To be rejected, a job must be submitted and not allocated yet.",
               job->id.to_cstring());

    job->state = JobState::JOB_STATE_REJECTED;
    data->nb_completed_jobs++;

    XBT_INFO("Job '%s' has been rejected", job->id.to_cstring());

    data->context->jobs_tracer.write_job(job);
    data->jobs_to_be_deleted.push_back(message->job_id);
}

void server_on_kill_jobs(ServerData * data,
                         IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<KillJobMessage *>(task_data->data);

    std::vector<JobIdentifier> jobs_ids_to_kill;

    for (const JobIdentifier & job_id : message->jobs_ids)
    {
        xbt_assert(data->context->workloads.job_is_registered(job_id),
                   "Trying to kill job '%s' but it does not exist.", job_id.to_cstring());

        auto job = data->context->workloads.job_at(job_id);

        // Let's discard jobs whose kill has already been requested
        if (!job->kill_requested)
        {
            // Let's check the job state
            xbt_assert(job->state == JobState::JOB_STATE_RUNNING || job->is_complete(),
                       "Invalid KILL_JOB: job_id '%s' refers to a job not being executed nor completed.",
                       job_id.to_cstring());

            // Let's mark that the job kill has been requested
            job->kill_requested = true;

            // The job is included in the killer_process arguments
            jobs_ids_to_kill.push_back(job_id);
        }
    }

    if (jobs_ids_to_kill.size() > 0)
    {
        simgrid::s4u::Actor::create("killer_process", simgrid::s4u::this_actor::get_host(),
                                    killer_process, data->context, jobs_ids_to_kill,
                                    JobState::JOB_STATE_COMPLETED_KILLED, true);
        ++data->nb_killers;
    }
}

void server_on_call_me_later(ServerData * data,
                             IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<CallMeLaterMessage *>(task_data->data);

    xbt_assert(message->target_time > simgrid::s4u::Engine::get_clock(),
               "You asked to be awaken in the past! (you ask: %f, it is: %f)",
               message->target_time, simgrid::s4u::Engine::get_clock());

    string pname = "waiter " + to_string(message->target_time);
    simgrid::s4u::Actor::create(pname.c_str(),
                                data->context->machines.master_machine()->host,
                                waiter_process, message->target_time, data);
    ++data->nb_waiters;
}

void server_on_execute_job(ServerData * data,
                           IPMessage * task_data)
{
    xbt_assert(task_data->data != nullptr);
    auto * message = static_cast<ExecuteJobMessage *>(task_data->data);
    auto * allocation = message->allocation;
    auto job = allocation->job;

    xbt_assert(job->state == JobState::JOB_STATE_SUBMITTED,
               "Cannot execute job '%s': its state (%s) is not JOB_STATE_SUBMITTED.",
               job->id.to_cstring(), job_state_to_string(job->state).c_str());

    job->state = JobState::JOB_STATE_RUNNING;

    data->nb_running_jobs++;
    xbt_assert(data->nb_running_jobs <= data->nb_submitted_jobs);

    if (!data->context->allow_compute_sharing || !data->context->allow_storage_sharing)
    {
        for (auto machine_id_it = allocation->machine_ids.elements_begin(); machine_id_it != allocation->machine_ids.elements_end(); ++machine_id_it)
        {
            int machine_id = *machine_id_it;
            const Machine * machine = data->context->machines[machine_id];
            if (machine->has_role(roles::Permissions::COMPUTE_NODE) && !data->context->allow_compute_sharing)
            {
                (void) machine; // Avoids a warning if assertions are ignored
                xbt_assert(machine->jobs_being_computed.empty(),
                           "Job '%s': Invalid allocation ('%s'): machine %d (hostname='%s') is currently computing jobs (these ones:"
                           " {%s}) whereas time-sharing on compute machines is disabled (rerun with --help to display the available options).",
                           job->id.to_cstring(),
                           allocation->machine_ids.to_string_hyphen().c_str(),
                           machine->id, machine->name.c_str(),
                           machine->jobs_being_computed_as_string().c_str());
            }
            if (machine->has_role(roles::Permissions::STORAGE) && !data->context->allow_storage_sharing)
            {
                (void) machine; // Avoids a warning if assertions are ignored
                xbt_assert(machine->jobs_being_computed.empty(),
                           "Job '%s': Invalid allocation ('%s'): machine %d (hostname='%s') is currently computing jobs (these ones:"
                           " {%s}) whereas time-sharing on storage machines is disabled (rerun with --help to display the available options).",
                           job->id.to_cstring(),
                           allocation->machine_ids.to_string_hyphen().c_str(),
                           machine->id, machine->name.c_str(),
                           machine->jobs_being_computed_as_string().c_str());
            }
        }
    }

    // Check that every machine can compute the job
    for (auto machine_id_it = allocation->machine_ids.elements_begin(); machine_id_it != allocation->machine_ids.elements_end(); ++machine_id_it)
    {
        int machine_id = *machine_id_it;
        Machine * machine = data->context->machines[machine_id];

        xbt_assert(machine->state == MachineState::COMPUTING || machine->state == MachineState::IDLE,
                   "Job '%s': Invalid job allocation ('%s'): machine %d (hostname='%s') cannot compute jobs now "
                   "(the machine is not computing nor idle, its state is '%s')",
                   job->id.to_cstring(),
                   allocation->machine_ids.to_string_hyphen().c_str(),
                   machine->id, machine->name.c_str(),
                   machine_state_to_string(machine->state).c_str());

        if (data->context->energy_used)
        {
        // Check that every machine is in a computation pstate
        int ps = machine->host->get_pstate();
        (void) ps; // Avoids a warning if assertions are ignored
        xbt_assert(machine->has_pstate(ps));
        xbt_assert(machine->pstates[ps] == PStateType::COMPUTATION_PSTATE,
                   "Job '%s': Invalid job allocation ('%s'): machine %d (hostname='%s') is not in a computation pstate (ps=%d)",
                   job->id.to_cstring(),
                   allocation->machine_ids.to_string_hyphen().c_str(),
                   machine->id, machine->name.c_str(), ps);
        }
    }

    // Only PARALLEL_HOMOGENEOUS_TOTAL_AMOUNT profile, or a sequence of those profile, are able to manage the following scenario:
    // The scheduler allocated a different number of resources than the number of requested resources.
    bool is_sequence_of_parhgtot = false;
    if (job->profile->type == ProfileType::SEQUENCE)
    {
        is_sequence_of_parhgtot = true;
        const auto & profile_seq = static_cast<SequenceProfileData *>(job->profile->data)->profile_sequence;
        for (const auto & subprofile : profile_seq)
        {
            if (subprofile->type != ProfileType::PARALLEL_HOMOGENEOUS_TOTAL_AMOUNT)
            {
                is_sequence_of_parhgtot = false;
            }
        }
    }

    if (!(job->profile->type == ProfileType::PARALLEL_HOMOGENEOUS_TOTAL_AMOUNT || is_sequence_of_parhgtot))
    {
        if (allocation->mapping.size() != 0)
        {
            xbt_assert(static_cast<unsigned int>(allocation->mapping.size()) == job->requested_nb_res,
                       "Job '%s' allocation ('%s') is invalid. The decision process set a custom mapping for this job, "
                       "but the custom mapping size (%zu) does not match the job requested number of machines (%d).",
                       job->id.to_cstring(),
                       allocation->machine_ids.to_string_hyphen().c_str(),
                       allocation->mapping.size(), job->requested_nb_res);
        }
        else
        {
            xbt_assert(static_cast<unsigned int>(allocation->machine_ids.size()) == job->requested_nb_res,
                       "Job '%s' allocation ('%s') is invalid. The job requires %d machines but only %u were given. "
                       "Using a different number of machines than the one requested is prevented by default. "
                       "If you meant to use multiple executors per machine, please specify a custom execution mapping "
                       "specifying which allocated machine each executor should use.",
                       job->id.to_cstring(),
                       allocation->machine_ids.to_string_hyphen().c_str(),
                       job->requested_nb_res, allocation->machine_ids.size());
        }
    }

    string pname = "job_" + job->id.to_string();
    auto actor = simgrid::s4u::Actor::create(pname.c_str(),
                                             data->context->machines[allocation->machine_ids.first_element()]->host,
                                             execute_job_process, data->context, allocation, true, message->io_profile);
    job->execution_actors.insert(actor);
}

bool is_simulation_finished(ServerData * data)
{
    return (data->submitter_counters[SubmitterType::JOB_SUBMITTER].nb_submitters_finished == data->submitter_counters[SubmitterType::JOB_SUBMITTER].expected_nb_submitters) &&
           (data->submitter_counters[SubmitterType::EVENT_SUBMITTER].nb_submitters_finished == data->submitter_counters[SubmitterType::EVENT_SUBMITTER].expected_nb_submitters) &&
           (!data->context->registration_sched_enabled || data->context->registration_sched_finished) && // Dynamic submissions are disabled or finished
           (data->nb_completed_jobs == data->nb_submitted_jobs) && // All submitted jobs have been completed (either computed and finished or rejected)
           (data->nb_running_jobs == 0) && // No jobs are being executed
           (data->nb_switching_machines == 0) && // No machine is being switched
           (data->nb_waiters == 0) && // No waiter process is running
           (data->nb_killers == 0); // No jobs is being killed
}

