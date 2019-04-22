/**
 * @file export.cpp
 * @brief Batsim's export classes and functions
 * @details Contains the classes and functions which are related to Batsim's exports.
 */

#include "export.hpp"

#include <algorithm>
#include <fstream>

#include <boost/algorithm/string/join.hpp>

#include <stdlib.h>
#include <xbt.h>
#include <math.h>
#include <float.h>

#include "context.hpp"
#include "jobs.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(export, "export"); //!< Logging


void prepare_batsim_outputs(BatsimContext * context)
{
    if (context->trace_schedule)
    {
        context->paje_tracer.set_filename(context->export_prefix + "_schedule.trace");
        context->machines.set_tracer(&context->paje_tracer);
        context->paje_tracer.initialize(context, MSG_get_clock());
    }

    if (context->trace_machine_states)
    {
        context->machine_state_tracer.set_context(context);
        context->machine_state_tracer.set_filename(context->export_prefix + "_machine_states.csv");
    }

    if (context->energy_used)
    {
        // Energy consumption tracing
        context->energy_tracer.set_context(context);
        context->energy_tracer.set_filename(context->export_prefix + "_consumed_energy.csv");

        // Power state tracing
        context->pstate_tracer.setFilename(context->export_prefix + "_pstate_changes.csv");

        std::map<int, IntervalSet> pstate_to_machine_set;
        for (const Machine * machine : context->machines.machines())
        {
            int machine_id = machine->id;
            int pstate = MSG_host_get_pstate(machine->host);

            if (pstate_to_machine_set.count(pstate) == 0)
            {
                IntervalSet range;
                range.insert(machine_id);
                pstate_to_machine_set[pstate] = range;
            }
            else
            {
                pstate_to_machine_set[pstate].insert(machine_id);
            }
        }

        for (auto mit : pstate_to_machine_set)
        {
            int pstate = mit.first;
            IntervalSet & range = mit.second;
            context->pstate_tracer.add_pstate_change(MSG_get_clock(), range, pstate);
        }
    }
}

void finalize_batsim_outputs(BatsimContext * context)
{
    // Let's say the simulation is ended now
    context->simulation_end_time = chrono::high_resolution_clock::now();

    // Schedule (Pajé)
    if (context->trace_schedule)
    {
        context->paje_tracer.finalize(context, MSG_get_clock());
    }

    if (context->trace_machine_states)
    {
        context->machine_state_tracer.flush();
        context->machine_state_tracer.close_buffer();
    }

    // Energy-related output
    if (context->energy_used)
    {
        context->energy_tracer.flush();
        context->pstate_tracer.flush();

        context->energy_tracer.close_buffer();
        context->pstate_tracer.close_buffer();
    }

    // Schedule-oriented output information
    export_schedule_to_csv(context->export_prefix + "_schedule.csv", context);

    // Job-oriented output information
    export_jobs_to_csv(context->export_prefix + "_jobs.csv", context);
}


WriteBuffer::WriteBuffer(const std::string & filename, int buffer_size)
    : buffer_size(buffer_size)
{
    xbt_assert(buffer_size > 0, "Invalid buffer size (%d)", buffer_size);
    buffer = new char[buffer_size];

    f.open(filename, ios_base::trunc);
    xbt_assert(f.is_open(), "Cannot write file '%s'", filename.c_str());
}

WriteBuffer::~WriteBuffer()
{
    if (buffer_pos > 0)
    {
        flush_buffer();
    }

    if (buffer != nullptr)
    {
        delete[] buffer;
        buffer = nullptr;

        f.close();
    }
}

void WriteBuffer::append_text(const char * text)
{
    const int text_length = strlen(text);

    // Is the buffer big enough?
    if (buffer_pos + text_length < buffer_size)
    {
        // Append the text into the buffer
        memcpy(buffer + buffer_pos, text, text_length * sizeof(char));
        buffer_pos += text_length;
    }
    else
    {
        // Write the current buffer content in the file
        flush_buffer();

        // Does the text fit in the (now empty) buffer?
        if (text_length < buffer_size)
        {
            // Copy the text into the buffer
            memcpy(buffer, text, text_length * sizeof(char));
            buffer_pos = text_length;
        }
        else
        {
            // Directly write the text into the file
            f.write(text, text_length);
        }
    }
}

void WriteBuffer::flush_buffer()
{
    f.write(buffer, buffer_pos);
    buffer_pos = 0;
}





PajeTracer::PajeTracer(bool log_launchings) :
    _log_launchings(log_launchings)
{
    generate_colors(64);
    shuffle_colors();
}

void PajeTracer::set_filename(const string &filename)
{
    xbt_assert(_wbuf == nullptr, "Double call of PajeTracer::set_filename");
    _wbuf = new WriteBuffer(filename);
}

PajeTracer::~PajeTracer()
{
    // If the write buffer had not been set, the PajeTracer has not been used and can disappear in silence
    if (_wbuf != nullptr)
    {
        if (state != FINALIZED && state != UNINITIALIZED)
        {
            fprintf(stderr, "Destruction of a PajeTracer object which has not been finalized. "
                            "The corresponding trace file may be invalid.\n");
        }

        if (_wbuf != nullptr)
        {
            delete _wbuf;
            _wbuf = nullptr;
        }
    }
}

void PajeTracer::initialize(const BatsimContext *context, double time)
{
    xbt_assert(state == UNINITIALIZED, "Bad PajeTracer::initialize call: the object is not UNINITIALIZED");

    // The buffer used to write lines in the file
    const int buf_size = 8 * 1024;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Coudln't allocate memory");

    // Let's write the Pajé schedule header
    // Let's write the Pajé schedule header
    nb_printed = snprintf(buf, buf_size,
                          "%%EventDef PajeDefineContainerType %d\n"
                          "%% Type string\n"
                          "%% Alias string\n"
                          "%% Name string\n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeCreateContainer %d\n"
                          "%% Time date \n"
                          "%% Type string  \n"
                          "%% Alias string \n"
                          "%% Name string   \n"
                          "%% Container string  \n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeDestroyContainer %d\n"
                          "%% Time date\n"
                          "%% Name string \n"
                          "%% Type string\n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeDefineStateType %d\n"
                          "%% Alias string \n"
                          "%% Type string \n"
                          "%% Name string \n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeDefineEntityValue %d\n"
                          "%% Alias string  \n"
                          "%% Type string  \n"
                          "%% Name string  \n"
                          "%% Color color \n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeSetState %d\n"
                          "%% Time date  \n"
                          "%% Type string  \n"
                          "%% Container string  \n"
                          "%% Value string  \n"
                          "%%EndEventDef \n"
                          "\n"
                          "%%EventDef PajeDefineEventType %d\n"
                          "%% Type string\n"
                          "%% Alias string\n"
                          "%% Name string\n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeNewEvent %d\n"
                          "%% Time date\n"
                          "%% Type string\n"
                          "%% Container string\n"
                          "%% Value string\n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeDefineVariableType %d\n"
                          "%% Type string\n"
                          "%% Alias string\n"
                          "%% Name string\n"
                          "%% Color string\n"
                          "%%EndEventDef\n"
                          "\n"
                          "%%EventDef PajeSetVariable %d\n"
                          "%% Time date\n"
                          "%% Type string\n"
                          "%% Container string\n"
                          "%% Value double\n"
                          "%%EndEventDef\n"
                          "\n",
                          DEFINE_CONTAINER_TYPE, CREATE_CONTAINER, DESTROY_CONTAINER,
                          DEFINE_STATE_TYPE, DEFINE_ENTITY_VALUE, SET_STATE,
                          DEFINE_EVENT_TYPE, NEW_EVENT, DEFINE_VARIABLE_TYPE,
                          SET_VARIABLE);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's create our container types
    nb_printed = snprintf(buf, buf_size,
                          "# Container types creation\n"
                          "%d 0 %s \"Machines\"\n"
                          "%d %s %s \"Machine\"\n"
                          "%d 0 %s \"Scheduler\"\n"
                          "%d %s %s \"Killer\"\n"
                          "\n",
                          DEFINE_CONTAINER_TYPE,                rootType,
                          DEFINE_CONTAINER_TYPE, rootType,      machineType,
                          DEFINE_CONTAINER_TYPE,                schedulerType,
                          DEFINE_CONTAINER_TYPE, schedulerType, killerType);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's create our event types
    nb_printed = snprintf(buf, buf_size,
                          "# Event types creation\n"
                          "%d %s %s \"Job kill\"\n"
                          "%d %s %s \"Job kill\"\n"
                          "\n",
                          DEFINE_EVENT_TYPE, killerType, killEventKiller,
                          DEFINE_EVENT_TYPE, machineType, killEventMachine);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's create our variable types
    nb_printed = snprintf(buf, buf_size,
                          "# Variable types creation\n"
                          "%d %s %s \"Utilization\" %s\n"
                          "\n",
                          DEFINE_VARIABLE_TYPE, schedulerType, utilizationVarType, utilizationColor);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    nb_printed = snprintf(buf, buf_size,
                          "# Containers creation\n"
                          "%d %lf %s %s \"Machines\" 0\n",
                          CREATE_CONTAINER, time, rootType, root);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    for (const Machine * m : context->machines.machines())
    {
        // todo : clean machine name
        nb_printed = snprintf(buf, buf_size,
                              "%d %lf %s %s%d \"%s\" %s\n",
                              CREATE_CONTAINER, time, machineType,
                              machinePrefix, m->id,
                              m->name.c_str(), root);
        xbt_assert(nb_printed < buf_size - 1,
                   "Writing error: buffer has been completely filled, some information might "
                   "have been lost. Please increase Batsim's output temporary buffers' size");
        _wbuf->append_text(buf);
    }

    nb_printed = snprintf(buf, buf_size,
                          "%d %lf %s %s \"Scheduler\" 0\n"
                          "%d %lf %s %s \"Killer\" %s\n"
                          "\n",
                          CREATE_CONTAINER, time, schedulerType, scheduler,
                          CREATE_CONTAINER, time, killerType, killer, scheduler);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's declare that machines have a state
    nb_printed = snprintf(buf, buf_size,
                          "# States creation\n"
                          "%d %s %s \"Machine state\"\n"
                          "\n",
                          DEFINE_STATE_TYPE, machineState, machineType);

    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's declare some machine states
    nb_printed = snprintf(buf, buf_size,
                          "# Creation of the different values the machine state can be\n"
                          "%d %s %s \"Waiting\" %s\n"
                          "%d %s %s \"Launching\" %s\n"
                          "\n"
                          "# Begin of events\n",
                          DEFINE_ENTITY_VALUE, mstateWaiting, machineState, waitingColor,
                          DEFINE_ENTITY_VALUE, mstateLaunching, machineState, launchingColor);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    // Let's set all the machines in waiting state
    for (const Machine * m : context->machines.machines())
    {
        nb_printed = snprintf(buf, buf_size,
                              "%d %lf %s %s%d %s\n",
                              SET_STATE, time, machineState, machinePrefix, m->id, mstateWaiting);
        xbt_assert(nb_printed < buf_size - 1,
                   "Writing error: buffer has been completely filled, some information might "
                   "have been lost. Please increase Batsim's output temporary buffers' size");
        _wbuf->append_text(buf);
    }

    free(buf);

    state = INITIALIZED;
}

void PajeTracer::finalize(const BatsimContext * context, double time)
{
    xbt_assert(state == INITIALIZED, "Bad PajeTracer::finalize call: the object has not been initialized yet");

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    nb_printed = snprintf(buf, buf_size,
                          "\n"
                          "# End of events, containers destruction\n");
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    for (const Machine * m : context->machines.machines())
    {
        nb_printed = snprintf(buf, buf_size,
                              "%d %lf %s%d %s\n",
                              DESTROY_CONTAINER, time, machinePrefix, m->id, machineType);
        xbt_assert(nb_printed < buf_size - 1,
                   "Writing error: buffer has been completely filled, some information might "
                   "have been lost. Please increase Batsim's output temporary buffers' size");
        _wbuf->append_text(buf);
    }

    nb_printed = snprintf(buf, buf_size,
                          "%d %lf %s %s\n",
                          DESTROY_CONTAINER, time, root, rootType);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);
    _wbuf->flush_buffer();

    free(buf);
    delete _wbuf;
    _wbuf = nullptr;

    state = FINALIZED;

    XBT_INFO("PajeTracer finalized");
}

void PajeTracer::add_job_launching(const Job * job,
                                   const std::vector<int> & used_machine_ids,
                                   double time)
{
    (void) job;
    xbt_assert(state == INITIALIZED, "Bad addJobLaunching call: the PajeTracer object is not initialized or had been finalized");

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    if (_log_launchings)
    {
        // Let's change the state of all the machines which launch the job
        for (const int & machineID : used_machine_ids)
        {
            nb_printed = snprintf(buf, buf_size,
                                  "%d %lf %s %s%d %s\n",
                                  SET_STATE, time, machineState, machinePrefix, machineID, mstateLaunching);
            xbt_assert(nb_printed < buf_size - 1,
                       "Writing error: buffer has been completely filled, some information might "
                       "have been lost. Please increase Batsim's output temporary buffers' size");
            _wbuf->append_text(buf);
        }
    }

    free(buf);
}

void PajeTracer::register_new_job(const Job *job)
{
    xbt_assert(_jobs.find(job) == _jobs.end(),
               "Cannot register new job %s: it already exists", job->id.to_string().c_str());

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    // Let's create a state value corresponding to this job
    nb_printed = snprintf(buf, buf_size,
                          "%d %s%s %s \"%s\" %s\n",
                          DEFINE_ENTITY_VALUE, jobPrefix, job->id.to_string().c_str(),
                          machineState, job->id.to_string().c_str(),
                          _colors[nb_total_jobs++ % (int)_colors.size()].c_str());
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    _jobs[job] = jobPrefix + job->id.to_string();

    free(buf);
}

void PajeTracer::set_machine_idle(int machine_id, double time)
{
    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    nb_printed = snprintf(buf, buf_size,
                          "%d %lf %s %s%d %s\n",
                          SET_STATE, time, machineState, machinePrefix, machine_id, mstateWaiting);
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    free(buf);
}

void PajeTracer::set_machine_as_computing_job(int machine_id, const Job * job, double time)
{
    auto mit = _jobs.find(job);
    if (mit == _jobs.end())
    {
        register_new_job(job);
        mit = _jobs.find(job);
    }

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    nb_printed = snprintf(buf, buf_size,
                          "%d %lf %s %s%d %s\n",
                          SET_STATE, time, machineState, machinePrefix, machine_id,
                          mit->second.c_str());
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    free(buf);
}

void PajeTracer::add_job_kill(const Job *job, const IntervalSet & used_machine_ids,
                              double time, bool associate_kill_to_machines)
{
    xbt_assert(state == INITIALIZED, "Bad addJobKill call: the PajeTracer object is not initialized or had been finalized");

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    // Let's add a kill event associated with the scheduler
    nb_printed = snprintf(buf, buf_size,
                          "%d %lf %s %s \"%s\"\n",
                          NEW_EVENT, time, killEventKiller, killer, job->id.to_string().c_str());
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(buf);

    if (associate_kill_to_machines)
    {
        // Let's add a kill event associated with each machine
        for (auto it = used_machine_ids.elements_begin(); it != used_machine_ids.elements_end(); ++it)
        {
            int machine_id = *it;
            nb_printed = snprintf(buf, buf_size,
                                  "%d %lf %s %s%d \"%s\"\n",
                                  NEW_EVENT, time, killEventMachine, machinePrefix, machine_id,
                                  job->id.to_string().c_str());
            xbt_assert(nb_printed < buf_size - 1,
                       "Writing error: buffer has been completely filled, some information might "
                       "have been lost. Please increase Batsim's output temporary buffers' size");
            _wbuf->append_text(buf);
        }
    }

    free(buf);
}

void PajeTracer::generate_colors(int color_count)
{
    xbt_assert(color_count > 0);

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    double hueFraction = 360.0 / color_count;
    for (int i = 0; i < color_count; ++i)
    {
        double h = i * hueFraction;
        double s=1, v=1, r, g, b;
        hsv_to_rgb(h,s,v, r,g,b);

        nb_printed = snprintf(buf, buf_size, "\"%lf %lf %lf\"", r, g, b);
        xbt_assert(nb_printed < buf_size - 1,
                   "Writing error: buffer has been completely filled, some information might "
                   "have been lost. Please increase Batsim's output temporary buffers' size");
        _colors.push_back(buf);
    }

    free(buf);
}

void PajeTracer::shuffle_colors()
{
    random_shuffle(_colors.begin(), _colors.end());
}

void PajeTracer::hsv_to_rgb(double h, double s, double v, double & r, double & g, double & b)
{
    if (s == 0) // Achromatic (grey)
    {
        r = g = b = v;
        return;
    }

    h /= 60;            // sector 0 to 5
    int i = floor(h);
    float f = h-i;      // factorial part of h
    float p = v*(1-s);
    float q = v*(1-s*f);
    float t = v*(1-s*(1-f));

    switch(i)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    default:    // case 5:
        r = v;
        g = p;
        b = q;
        break;
    }
}

void export_jobs_to_csv(const std::string &filename, const BatsimContext *context)
{
    ofstream f(filename, ios_base::trunc);
    xbt_assert(f.is_open(), "Cannot write file '%s'", filename.c_str());

    // List all features (columns)
    map<string, string> job_map;
    vector<string> key_list = {
        "job_id",
        "workload_name",
        "profile",
        "submission_time",
        "requested_number_of_resources",
        "requested_time",
        "success",
        "final_state",
        "starting_time",
        "execution_time",
        "finish_time",
        "waiting_time",
        "turnaround_time",
        "stretch",
        "allocated_resources",
        "consumed_energy",
        "metadata"
    };

    // Write headers (columns) to the output file
    vector<string> row_content;
    row_content.reserve(job_map.size());

    for (string & mit : key_list)
    {
        row_content.push_back(mit);
    }

    f << boost::algorithm::join(row_content, ",") << "\n";

    for (const auto mit : context->workloads.workloads())
    {
        string workload_name = mit.first;
        const Workload * workload = mit.second;

        if(workload->jobs)
        {
            const auto & jobs = workload->jobs->jobs();
            for (const auto & mit : jobs)
            {
                Job * job = mit.second;

                int success = (job->state == JobState::JOB_STATE_COMPLETED_SUCCESSFULLY);
                bool rejected = (job->state == JobState::JOB_STATE_REJECTED);

                // Update all values
                job_map["job_id"] = job->id.job_name;
                job_map["workload_name"] = string(workload_name);
                job_map["profile"] = job->profile;
                job_map["submission_time"] = to_string((double)job->submission_time);
                job_map["requested_number_of_resources"] = to_string(job->requested_nb_res);
                job_map["requested_time"] = to_string((double)job->walltime);
                job_map["success"] = to_string(success);
                job_map["final_state"] = job_state_to_string(job->state);
                job_map["starting_time"] = rejected ? "" : to_string((double)job->starting_time);
                job_map["execution_time"] = rejected ? "" :to_string((double)job->runtime);
                job_map["finish_time"] = rejected ? "" :to_string((double)(job->starting_time + job->runtime));
                job_map["waiting_time"] = rejected ? "" :to_string((double)(job->starting_time - job->submission_time));
                job_map["turnaround_time"] = rejected ? "" :to_string((double)(job->starting_time + job->runtime - job->submission_time));
                job_map["stretch"] = rejected ? "" :to_string((double)((job->starting_time + job->runtime - job->submission_time) / job->runtime));
                job_map["consumed_energy"] = rejected ? "" :to_string(job->consumed_energy);
                job_map["allocated_resources"] = job->allocation.to_string_hyphen(" ");
                job_map["metadata"] = '"' + job->metadata + '"';


                // Write values to the output file
                row_content.resize(0);
                for (string & mit : key_list)
                {
                    row_content.push_back(job_map[mit]);
                }

                f << boost::algorithm::join(row_content, ",") << "\n";
            }
        }
    }

    f.close();
}


void export_schedule_to_csv(const std::string &filename, const BatsimContext *context)
{
    ofstream f(filename, ios_base::trunc);
    xbt_assert(f.is_open(), "Cannot write file '%s'", filename.c_str());

    int nb_jobs = 0;
    int nb_jobs_finished = 0;
    int nb_jobs_success = 0;
    int nb_jobs_killed = 0;
    long double makespan = 0;
    long double sum_waiting_time = 0;
    long double sum_turnaround_time = 0;
    long double sum_slowdown = 0;
    long double max_waiting_time = 0;
    long double max_turnaround_time = 0;
    long double max_slowdown = 0;

    map<string, string> output_map;

    map<int, long double> machines_utilisation;
    for (int i=0; i<context->machines.nb_machines(); i++)
    {
        machines_utilisation[i] = 0;
    }

    long double seconds_used_by_scheduler = context->microseconds_used_by_scheduler / 1e6l;
    output_map["scheduling_time"] = to_string((double) seconds_used_by_scheduler);

    // Let's compute the simulation time
    chrono::duration<long double> diff = context->simulation_end_time - context->simulation_start_time;
    long double seconds_used_by_the_whole_simulation = diff.count();
    output_map["simulation_time"] = to_string((double) seconds_used_by_the_whole_simulation);

    // Let's compute jobs-oriented metrics
    for (const auto mit : context->workloads.workloads())
    {
        const Workload * workload = mit.second;

        if(workload->jobs != nullptr)
        {
            const auto & jobs = workload->jobs->jobs();
            for (const auto & mit : jobs)
            {
                Job * job = mit.second;
                nb_jobs++;

                if (job->is_complete())
                {
                    nb_jobs_finished++;

                    if (job->state == JobState::JOB_STATE_COMPLETED_SUCCESSFULLY)
                    {
                        nb_jobs_success++;
                    }
                    else
                    {
                        nb_jobs_killed++;
                    }


                    long double starting_time = job->starting_time;
                    long double waiting_time = starting_time - job->submission_time;
                    long double completion_time = job->starting_time + job->runtime;
                    long double turnaround_time = completion_time - job->submission_time;
                    long double slowdown = turnaround_time / job->runtime;

                    sum_waiting_time += waiting_time;
                    sum_turnaround_time += turnaround_time;
                    sum_slowdown += slowdown;

                    if (completion_time > makespan)
                    {
                        makespan = completion_time;
                    }

                    if (waiting_time > max_waiting_time)
                    {
                        max_waiting_time = waiting_time;
                    }

                    if (turnaround_time > max_turnaround_time)
                    {
                        max_turnaround_time = turnaround_time;
                    }

                    if (slowdown > max_slowdown)
                    {
                        max_slowdown = slowdown;
                    }

                    IntervalSet & allocation = job->allocation;
                    auto size = allocation.size();
                    for (size_t i=0; i < size; i++)
                    {
                        int machine_idx = allocation[i];
                        machines_utilisation[machine_idx] += job->runtime;
                    }
                }
            }
        }
    }
    double sum_time_running = 0;
    double max_time_running = 0;

    for (auto const & entry : machines_utilisation)
    {
        sum_time_running += (double) entry.second;
        if ((double) entry.second > max_time_running)
        {
            max_time_running = (double) entry.second;
        }
    }
    double mean_time_running = sum_time_running / machines_utilisation.size();

    double success_rate = (double)nb_jobs_success/nb_jobs;

    double mean_waiting_time = (double)sum_waiting_time/nb_jobs;
    double mean_turnaround_time = (double)sum_turnaround_time/nb_jobs;
    double mean_slowdown = (double)sum_slowdown/nb_jobs;

    output_map["batsim_version"] = context->batsim_version;
    output_map["nb_jobs"] = to_string(nb_jobs);
    output_map["nb_jobs_finished"] = to_string(nb_jobs_finished);
    output_map["nb_jobs_success"] = to_string(nb_jobs_success);
    output_map["nb_jobs_killed"] = to_string(nb_jobs_killed);
    output_map["nb_jobs_rejected"] = to_string(nb_jobs - nb_jobs_finished);
    output_map["success_rate"] = to_string(success_rate);

    output_map["makespan"] = to_string((double)makespan);
    output_map["mean_waiting_time"] = to_string(mean_waiting_time);
    output_map["mean_turnaround_time"] = to_string(mean_turnaround_time);
    output_map["mean_slowdown"] = to_string(mean_slowdown);
    output_map["max_waiting_time"] = to_string((double)max_waiting_time);
    output_map["max_turnaround_time"] = to_string((double)max_turnaround_time);
    output_map["max_slowdown"] = to_string((double)max_slowdown);

    output_map["nb_computing_machines"] = to_string(context->machines.nb_machines());

    XBT_INFO("jobs=%d, finished=%d, success=%d, killed=%d, success_rate=%lf",
             nb_jobs, nb_jobs_finished, nb_jobs_success, nb_jobs_killed, success_rate);
    XBT_INFO("makespan=%lf, scheduling_time=%lf, mean_waiting_time=%lf, mean_turnaround_time=%lf, "
             "mean_slowdown=%lf, max_waiting_time=%lf, max_turnaround_time=%lf, max_slowdown=%lf",
             (double)makespan, (double)seconds_used_by_scheduler,
             (double)mean_waiting_time, (double)mean_turnaround_time, mean_slowdown,
             (double)max_waiting_time, (double)max_turnaround_time, (double)max_slowdown);
    XBT_INFO("mean_machines_running=%lf, max_machines_running=%lf",
             (double)mean_time_running, (double)max_time_running);

    long double total_consumed_energy = context->energy_last_job_completion - context->energy_first_job_submission;
    output_map["consumed_joules"] = to_string((double) total_consumed_energy);

    output_map["nb_machine_switches"] = to_string(context->nb_machine_switches);
    output_map["nb_grouped_switches"] = to_string(context->nb_grouped_switches);

    // Let's compute machine-related metrics
    map<MachineState, long double> time_spent_in_each_state;
    const vector<MachineState> machine_states = {MachineState::SLEEPING, MachineState::IDLE,
                                                 MachineState::COMPUTING,
                                                 MachineState::TRANSITING_FROM_SLEEPING_TO_COMPUTING,
                                                 MachineState::TRANSITING_FROM_COMPUTING_TO_SLEEPING};
    for (const MachineState & state : machine_states)
    {
        time_spent_in_each_state[state] = 0;
    }

    for (const Machine * machine : context->machines.machines())
    {
        for (const MachineState & state : machine_states)
        {
            time_spent_in_each_state[state] += machine->time_spent_in_each_state.at(state);
        }
    }

    for (const MachineState & state : machine_states)
    {
        output_map["time_" + machine_state_to_string(state)] = to_string((double)time_spent_in_each_state[state]);
    }


    // Let's write the output map into the file
    vector<string> keys, values;
    for (const auto mit : output_map)
    {
        keys.push_back(mit.first);
        values.push_back(mit.second);
    }

    f << boost::algorithm::join(keys, ",") << "\n";
    f << boost::algorithm::join(values, ",") << "\n";

    f.close();
}

PStateChangeTracer::PStateChangeTracer()
{
    xbt_assert(_temporary_buffer == nullptr);
    _temporary_buffer = (char*) malloc(512 * sizeof(char));
}

void PStateChangeTracer::setFilename(const string &filename)
{
    xbt_assert(_wbuf == nullptr, "Double call of PStateChangeTracer::setFilename");
    _wbuf = new WriteBuffer(filename);

    _wbuf->append_text("time,machine_id,new_pstate\n");
}

PStateChangeTracer::~PStateChangeTracer()
{
    if (_wbuf != nullptr)
    {
        delete _wbuf;
        _wbuf = nullptr;
    }

    if (_temporary_buffer != nullptr)
    {
        free(_temporary_buffer);
        _temporary_buffer = nullptr;
    }
}

void PStateChangeTracer::add_pstate_change(double time, const IntervalSet & machines, int pstate_after)
{
    xbt_assert(_wbuf != nullptr);

    const string machines_as_string = machines.to_string_hyphen(" ", "-");
    const int minimum_buf_size = 256 + machines_as_string.size();
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored

    // Increases the buffer size if needed
    _temporary_buffer = (char*) realloc(_temporary_buffer, minimum_buf_size);
    xbt_assert(_temporary_buffer != 0, "Couldn't allocate memory!");

    nb_printed = snprintf(_temporary_buffer, minimum_buf_size, "%g,%s,%d\n",
                          time, machines_as_string.c_str(), pstate_after);
    xbt_assert(nb_printed < minimum_buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    _wbuf->append_text(_temporary_buffer);
}

void PStateChangeTracer::flush()
{
    xbt_assert(_wbuf != nullptr);

    _wbuf->flush_buffer();
}

void PStateChangeTracer::close_buffer()
{
    xbt_assert(_wbuf != nullptr);

    delete _wbuf;
    _wbuf = nullptr;
}



EnergyConsumptionTracer::~EnergyConsumptionTracer()
{
    if (_wbuf != nullptr)
    {
        delete _wbuf;
        _wbuf = nullptr;
    }
}

void EnergyConsumptionTracer::set_context(BatsimContext *context)
{
    xbt_assert(context->energy_used);
    _context = context;
}

void EnergyConsumptionTracer::set_filename(const string &filename)
{
    xbt_assert(_wbuf == nullptr, "Double call of EnergyConsumptionTracer::set_filename");
    _wbuf = new WriteBuffer(filename);

    _wbuf->append_text("time,energy,event_type,wattmin,epower\n");
}

void EnergyConsumptionTracer::add_job_start(double date, JobIdentifier job_id)
{
    (void) job_id;
    add_entry(date, 's');
}

void EnergyConsumptionTracer::add_job_end(double date, JobIdentifier job_id)
{
    (void) job_id;
    _context->energy_last_job_completion = add_entry(date, 'e');
}

void EnergyConsumptionTracer::add_pstate_change(double date, const IntervalSet & machines, int new_pstate)
{
    (void) machines;
    (void) new_pstate;
    add_entry(date, 'p');
}

void EnergyConsumptionTracer::flush()
{
    xbt_assert(_wbuf != nullptr);

    _wbuf->flush_buffer();
}

void EnergyConsumptionTracer::close_buffer()
{
    xbt_assert(_wbuf != nullptr);

    delete _wbuf;
    _wbuf = nullptr;
}

long double EnergyConsumptionTracer::add_entry(double date, char event_type)
{
    xbt_assert(_wbuf != nullptr);

    long double energy = _context->machines.total_consumed_energy(_context);
    long double wattmin = _context->machines.total_wattmin(_context);

    long double time_diff = (long double)date - _last_entry_date;
    long double energy_diff = energy - _last_entry_energy;
    long double epower = -1;

    if (time_diff > 0)
    {
        epower = energy_diff / time_diff;
    }

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    if (epower != -1)
    {
        nb_printed = snprintf(buf, buf_size, "%g,%Lg,%c,%Lg,%g\n",
                              date, energy, event_type, wattmin, (double)epower);
    }
    else
    {
        nb_printed = snprintf(buf, buf_size, "%g,%Lg,%c,%Lg,NA\n",
                              date, energy, event_type, wattmin);
    }
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    xbt_assert(_wbuf != nullptr);
    _wbuf->append_text(buf);

    free(buf);

    _last_entry_date = date;
    _last_entry_energy = energy;

    return energy;
}


MachineStateTracer::~MachineStateTracer()
{
    if (_wbuf != nullptr)
    {
        delete _wbuf;
        _wbuf = nullptr;
    }
}

void MachineStateTracer::set_context(BatsimContext *context)
{
    _context = context;
}

void MachineStateTracer::set_filename(const string &filename)
{
    xbt_assert(_wbuf == nullptr, "Double call of MachineStateTracer::set_filename");
    _wbuf = new WriteBuffer(filename);

    vector<string> header_substrings;
    const vector<MachineState> machine_states = {MachineState::SLEEPING,
                                                 MachineState::TRANSITING_FROM_SLEEPING_TO_COMPUTING,
                                                 MachineState::TRANSITING_FROM_COMPUTING_TO_SLEEPING,
                                                 MachineState::IDLE,
                                                 MachineState::COMPUTING};
    for (const MachineState & state : machine_states)
    {
        header_substrings.push_back("nb_" + machine_state_to_string(state));
    }

    string header = "time," + boost::algorithm::join(header_substrings, ",") + "\n";

    _wbuf->append_text(header.c_str());
    _wbuf->flush_buffer();
}

void MachineStateTracer::write_machine_states(double date)
{
    xbt_assert(_context != nullptr);
    xbt_assert(_wbuf != nullptr);

    const std::map<MachineState, int> & numbers = _context->machines.nb_machines_in_each_state();

    const int buf_size = 256;
    int nb_printed;
    (void) nb_printed; // Avoids a warning if assertions are ignored
    char * buf = (char*) malloc(sizeof(char) * buf_size);
    xbt_assert(buf != 0, "Couldn't allocate memory");

    nb_printed = snprintf(buf, buf_size, "%g,%d,%d,%d,%d,%d\n",
                          date,
                          numbers.at(MachineState::SLEEPING),
                          numbers.at(MachineState::TRANSITING_FROM_SLEEPING_TO_COMPUTING),
                          numbers.at(MachineState::TRANSITING_FROM_COMPUTING_TO_SLEEPING),
                          numbers.at(MachineState::IDLE),
                          numbers.at(MachineState::COMPUTING));
    xbt_assert(nb_printed < buf_size - 1,
               "Writing error: buffer has been completely filled, some information might "
               "have been lost. Please increase Batsim's output temporary buffers' size");
    xbt_assert(_wbuf != nullptr);
    _wbuf->append_text(buf);

    free(buf);
}

void MachineStateTracer::flush()
{
    xbt_assert(_wbuf != nullptr);

    _wbuf->flush_buffer();
}

void MachineStateTracer::close_buffer()
{
    xbt_assert(_wbuf != nullptr);

    delete _wbuf;
    _wbuf = nullptr;
}
